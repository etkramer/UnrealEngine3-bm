/*=============================================================================
	UnParticleComponent.cpp: Particle component implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "LevelUtils.h"
#include "UnNovodexSupport.h"
#include "ImageUtils.h"
#include "EngineSequenceClasses.h"

IMPLEMENT_CLASS(AEmitter);
IMPLEMENT_CLASS(AEmitterCameraLensEffectBase);

IMPLEMENT_CLASS(UParticleSystem);
IMPLEMENT_CLASS(UParticleEmitter);
IMPLEMENT_CLASS(UParticleSpriteEmitter);
IMPLEMENT_CLASS(UParticleSystemComponent);
IMPLEMENT_CLASS(UParticleLODLevel);
IMPLEMENT_CLASS(UParticleSystemReplay);


/** Whether to allow particle systems to perform work. */
UBOOL GIsAllowingParticles = TRUE;

// Comment this in to debug empty emitter instance templates...
//#define _PSYSCOMP_DEBUG_INVALID_EMITTER_INSTANCE_TEMPLATES_

/*-----------------------------------------------------------------------------
	Particle scene view
-----------------------------------------------------------------------------*/
FSceneView*			GParticleView = NULL;

/*-----------------------------------------------------------------------------
	Particle data manager
-----------------------------------------------------------------------------*/
FParticleDataManager	GParticleDataManager;


/*-----------------------------------------------------------------------------
	Detailed particle tick stats.
-----------------------------------------------------------------------------*/

#define TRACK_DETAILED_PARTICLE_TICK_STATS (!FINAL_RELEASE)

#if TRACK_DETAILED_PARTICLE_TICK_STATS

/**
 * Tick time stats for a particle system.
 */
struct FParticleTickStats
{
	/** Total accumulative tick time in seconds. */
	FLOAT	AccumTickTime;
	/** Max tick time in seconds. */
	FLOAT	MaxTickTime;
	/** Total accumulative active particles. */
	INT		AccumActiveParticles;
	/** Max active particle count. */
	INT		MaxActiveParticleCount;
	/** Total tick count. */
	INT		TickCount;

	/** Constructor, initializing all member variables. */
	FParticleTickStats()
	:	AccumTickTime(0)
	,	MaxTickTime(0)
	,	AccumActiveParticles(0)
	,	MaxActiveParticleCount(0)
	,	TickCount(0)
	{
	}
};

/**
 * Per particle system (template) tick time stats tracking.
 */
class FParticleTickStatManager : FSelfRegisteringExec
{
public:
	/** Constructor, initializing all member variables. */
	FParticleTickStatManager()
	:	bIsEnabled(FALSE)
	{}

	/**
	 * Exec handler routed via UObject::StaticExec
	 *
	 * @param	Cmd		Command to parse
	 * @param	Ar		Output device to log to
	 *
	 * @return	TRUE if command was handled, FALSE otherwise
	 */
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar )
	{
		const TCHAR *Str = Cmd;
		if( ParseCommand(&Str,TEXT("PARTICLETICKSTATS")) )
		{
			// Empty out stats.
			if( ParseCommand(&Str,TEXT("RESET")) )
			{
				ParticleSystemToTickStatsMap.Empty();
				return TRUE;
			}
			// Start tracking.
			else if( ParseCommand(&Str,TEXT("START")) )
			{
				bIsEnabled = TRUE;
				return TRUE;
			}
			// Stop tracking
			else if( ParseCommand(&Str,TEXT("STOP")) )
			{
				bIsEnabled = FALSE;
				return TRUE;
			}
			// Dump stats in CSV file format to log.
			else if( ParseCommand(&Str,TEXT("DUMP")) )
			{
				// Header for CSV format.
				Ar.Logf(TEXT(",avg ms/ tick, max ms/ tick, ticks, avg active/ tick, max active/ tick, particle system"));

				// Iterate over all gathered stats and dump them (unsorted) in CSV file format, pasteable into Excel.
				for( TMap<FString,FParticleTickStats>::TConstIterator It(ParticleSystemToTickStatsMap); It; ++It )
				{
					const FString& ParticleSystemName = It.Key();
					const FParticleTickStats& TickStats = It.Value();
					Ar.Logf(TEXT(", %5.2f, %5.2f, %i, %i, %i, %s"), 
						TickStats.AccumTickTime / TickStats.TickCount * 1000.f,
						TickStats.MaxTickTime * 1000.f,
						TickStats.TickCount,
						TickStats.AccumActiveParticles / TickStats.TickCount,
						TickStats.MaxActiveParticleCount,
						*ParticleSystemName );
				}
				return TRUE;
			}
			return FALSE;
		}
		return FALSE;
	}

	/**
	 * Updates stats with passed in information.
	 *
	 * @param	Component			Particle system component to update associated template's stat for
	 * @param	TickTime			Time it took to tick this component
	 * @param	ActiveParticles		Number of currently active particles
	 */
	void UpdateStats( UParticleSystem* Template, FLOAT TickTime, INT ActiveParticles )
	{
		check(Template);

		// Find existing entry and update it if found.
		FParticleTickStats* ParticleTickStats = ParticleSystemToTickStatsMap.Find( Template->GetPathName() );
		if( ParticleTickStats )
		{
			ParticleTickStats->AccumTickTime += TickTime;
			ParticleTickStats->MaxTickTime = Max( ParticleTickStats->MaxTickTime, TickTime );
			ParticleTickStats->AccumActiveParticles += ActiveParticles;
			ParticleTickStats->MaxActiveParticleCount = Max( ParticleTickStats->MaxActiveParticleCount, ActiveParticles );
			ParticleTickStats->TickCount++;
		}
		// Create new entry.
		else
		{
			// Create new entry...
			FParticleTickStats NewParticleTickStats;
			NewParticleTickStats.AccumTickTime = TickTime;
			NewParticleTickStats.MaxTickTime = TickTime;
			NewParticleTickStats.AccumActiveParticles = ActiveParticles;
			NewParticleTickStats.MaxActiveParticleCount = ActiveParticles;
			NewParticleTickStats.TickCount = 1;
			// ... and set it for subsequent retrieval.
			ParticleSystemToTickStatsMap.Set( *Template->GetPathName(), NewParticleTickStats );
		}
	}

	/** Mapping from particle system to tick stats. */
	TMap<FString,FParticleTickStats> ParticleSystemToTickStatsMap;

	/** Whether tracking is currently enabled. */
	UBOOL bIsEnabled;
};

/** Global tick stat manager object. */
FParticleTickStatManager* GParticleTickStatManager = NULL;

#endif

/*-----------------------------------------------------------------------------
	Conversion functions
-----------------------------------------------------------------------------*/
void Particle_ModifyFloatDistribution(UDistributionFloat* pkDistribution, FLOAT fScale)
{
	if (pkDistribution->IsA(UDistributionFloatConstant::StaticClass()))
	{
		UDistributionFloatConstant* pkDistConstant = Cast<UDistributionFloatConstant>(pkDistribution);
		pkDistConstant->Constant *= fScale;
	}
	else
	if (pkDistribution->IsA(UDistributionFloatUniform::StaticClass()))
	{
		UDistributionFloatUniform* pkDistUniform = Cast<UDistributionFloatUniform>(pkDistribution);
		pkDistUniform->Min *= fScale;
		pkDistUniform->Max *= fScale;
	}
	else
	if (pkDistribution->IsA(UDistributionFloatConstantCurve::StaticClass()))
	{
		UDistributionFloatConstantCurve* pkDistCurve = Cast<UDistributionFloatConstantCurve>(pkDistribution);

		INT iKeys = pkDistCurve->GetNumKeys();
		INT iCurves = pkDistCurve->GetNumSubCurves();

		for (INT KeyIndex = 0; KeyIndex < iKeys; KeyIndex++)
		{
			FLOAT fKeyIn = pkDistCurve->GetKeyIn(KeyIndex);
			for (INT SubIndex = 0; SubIndex < iCurves; SubIndex++)
			{
				FLOAT fKeyOut = pkDistCurve->GetKeyOut(SubIndex, KeyIndex);
				FLOAT ArriveTangent;
				FLOAT LeaveTangent;
                pkDistCurve->GetTangents(SubIndex, KeyIndex, ArriveTangent, LeaveTangent);

				pkDistCurve->SetKeyOut(SubIndex, KeyIndex, fKeyOut * fScale);
				pkDistCurve->SetTangents(SubIndex, KeyIndex, ArriveTangent * fScale, LeaveTangent * fScale);
			}
		}
	}
}

void Particle_ModifyVectorDistribution(UDistributionVector* pkDistribution, FVector& vScale)
{
	if (pkDistribution->IsA(UDistributionVectorConstant::StaticClass()))
	{
		UDistributionVectorConstant* pkDistConstant = Cast<UDistributionVectorConstant>(pkDistribution);
		pkDistConstant->Constant *= vScale;
	}
	else
	if (pkDistribution->IsA(UDistributionVectorUniform::StaticClass()))
	{
		UDistributionVectorUniform* pkDistUniform = Cast<UDistributionVectorUniform>(pkDistribution);
		pkDistUniform->Min *= vScale;
		pkDistUniform->Max *= vScale;
	}
	else
	if (pkDistribution->IsA(UDistributionVectorConstantCurve::StaticClass()))
	{
		UDistributionVectorConstantCurve* pkDistCurve = Cast<UDistributionVectorConstantCurve>(pkDistribution);

		INT iKeys = pkDistCurve->GetNumKeys();
		INT iCurves = pkDistCurve->GetNumSubCurves();

		for (INT KeyIndex = 0; KeyIndex < iKeys; KeyIndex++)
		{
			FLOAT fKeyIn = pkDistCurve->GetKeyIn(KeyIndex);
			for (INT SubIndex = 0; SubIndex < iCurves; SubIndex++)
			{
				FLOAT fKeyOut = pkDistCurve->GetKeyOut(SubIndex, KeyIndex);
				FLOAT ArriveTangent;
				FLOAT LeaveTangent;
                pkDistCurve->GetTangents(SubIndex, KeyIndex, ArriveTangent, LeaveTangent);

				switch (SubIndex)
				{
				case 1:
					pkDistCurve->SetKeyOut(SubIndex, KeyIndex, fKeyOut * vScale.Y);
					pkDistCurve->SetTangents(SubIndex, KeyIndex, ArriveTangent * vScale.Y, LeaveTangent * vScale.Y);
					break;
				case 2:
					pkDistCurve->SetKeyOut(SubIndex, KeyIndex, fKeyOut * vScale.Z);
					pkDistCurve->SetTangents(SubIndex, KeyIndex, ArriveTangent * vScale.Z, LeaveTangent * vScale.Z);
					break;
				case 0:
				default:
					pkDistCurve->SetKeyOut(SubIndex, KeyIndex, fKeyOut * vScale.X);
					pkDistCurve->SetTangents(SubIndex, KeyIndex, ArriveTangent * vScale.X, LeaveTangent * vScale.X);
					break;
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	AEmitter implementation.
-----------------------------------------------------------------------------*/
void AEmitter::Spawned()
{
	Super::Spawned();

	if (ParticleSystemComponent && bPostUpdateTickGroup)
	{
		ParticleSystemComponent->SetTickGroup(TG_PostUpdateWork);
	}
}

void AEmitter::PostBeginPlay()
{
	Super::PostBeginPlay();

	if (ParticleSystemComponent && bPostUpdateTickGroup)
	{
		ParticleSystemComponent->SetTickGroup(TG_PostUpdateWork);
	}
}


/** 
 *	ticks the actor
 *	@param	DeltaTime	The time slice of this tick
 *	@param	TickType	The type of tick that is happening
 *
 *	@return	TRUE if the actor was ticked, FALSE if it was aborted (e.g. because it's in stasis)
 */
UBOOL AEmitter::Tick( FLOAT DeltaTime, enum ELevelTick TickType )
{
	UBOOL bResult = Super::Tick(DeltaTime, TickType);
	if (bResult)
	{
		// Process events here...
		if ((GeneratedEvents.Num() > 0) && (ParticleSystemComponent != NULL))
		{
			TArray<INT> ActivatedIndices;
			ActivatedIndices.Empty();
			ActivatedIndices.AddZeroed(1);
			for (INT GenEventIndex = 0; GenEventIndex < GeneratedEvents.Num(); GenEventIndex++)
			{
				USeqEvent_ParticleEvent* ParticleEvent = Cast<USeqEvent_ParticleEvent>(GeneratedEvents(GenEventIndex));
				if (ParticleEvent != NULL)
				{
					for (INT OutputIndex = 0; OutputIndex < ParticleEvent->OutputLinks.Num(); OutputIndex++)
					{
						FSeqOpOutputLink& SOOL = ParticleEvent->OutputLinks(OutputIndex);
						for (INT SpawnIndex = 0; SpawnIndex < ParticleSystemComponent->SpawnEvents.Num(); SpawnIndex++)
						{
							FParticleEventSpawnData& SpawnEvent = ParticleSystemComponent->SpawnEvents(SpawnIndex);
							if (SOOL.LinkDesc == SpawnEvent.EventName.ToString())
							{
								ActivatedIndices(0) = OutputIndex;
								// Set the data and activate the event...
								ParticleEvent->EventType = ePARTICLEOUT_Spawn;
								ParticleEvent->EventPosition = SpawnEvent.Location;
								ParticleEvent->EventVelocity = SpawnEvent.Velocity;
								ParticleEvent->EventEmitterTime = SpawnEvent.EmitterTime;
								ParticleEvent->CheckActivate(this, NULL, FALSE, &ActivatedIndices);
							}
						}
						for (INT DeathIndex = 0; DeathIndex < ParticleSystemComponent->DeathEvents.Num(); DeathIndex++)
						{
							FParticleEventDeathData& DeathEvent = ParticleSystemComponent->DeathEvents(DeathIndex);
							if (SOOL.LinkDesc == DeathEvent.EventName.ToString())
							{
								ActivatedIndices(0) = OutputIndex;
								// Set the data and activate the event...
								ParticleEvent->EventType = ePARTICLEOUT_Death;
								ParticleEvent->EventPosition = DeathEvent.Location;
								ParticleEvent->EventVelocity = DeathEvent.Velocity;
								ParticleEvent->EventEmitterTime = DeathEvent.EmitterTime;
								ParticleEvent->EventParticleTime = DeathEvent.ParticleTime;
								ParticleEvent->CheckActivate(this, NULL, FALSE, &ActivatedIndices);
							}
						}
						for (INT CollisionIndex = 0; CollisionIndex < ParticleSystemComponent->CollisionEvents.Num(); CollisionIndex++)
						{
							FParticleEventCollideData& CollisionEvent = ParticleSystemComponent->CollisionEvents(CollisionIndex);
							if (SOOL.LinkDesc == CollisionEvent.EventName.ToString())
							{
								ActivatedIndices(0) = OutputIndex;
								// Set the data and activate the event...
								ParticleEvent->EventType = ePARTICLEOUT_Collision;
								ParticleEvent->EventPosition = CollisionEvent.Location;
								ParticleEvent->EventVelocity = CollisionEvent.Velocity;
								ParticleEvent->EventEmitterTime = CollisionEvent.EmitterTime;
								ParticleEvent->EventParticleTime = CollisionEvent.ParticleTime;
								if (ParticleEvent->UseRelfectedImpactVector == TRUE)
								{
									ParticleEvent->EventNormal = CollisionEvent.Direction.MirrorByVector(CollisionEvent.Normal);
								}
								else
								{
									ParticleEvent->EventNormal = CollisionEvent.Normal;
								}
								ParticleEvent->CheckActivate(this, NULL, FALSE, &ActivatedIndices);
							}
						}

						// NOTE: We do NOT process kismet-generated events the same way...
						// I.e., there is no support for Kismet-->Emitter-->Kismet event passing.
					}
				}
			}
		}
	}

	return bResult;
}

void AEmitter::SetTemplate(UParticleSystem* NewTemplate, UBOOL bDestroyOnFinish)
{
	if (ParticleSystemComponent)
	{
		FComponentReattachContext ReattachContext(ParticleSystemComponent);
		ParticleSystemComponent->SetTemplate(NewTemplate);
		if (bPostUpdateTickGroup)
		{
			ParticleSystemComponent->SetTickGroup(TG_PostUpdateWork);
		}
	}
	bDestroyOnSystemFinish = bDestroyOnFinish;
}

void AEmitter::AutoPopulateInstanceProperties()
{
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->AutoPopulateInstanceProperties();
	}
}

void AEmitter::CheckForErrors()
{
	Super::CheckForErrors();

	// Emitters placed in a level should have a non-NULL ParticleSystemComponent.
	UObject* Outer = GetOuter();
	if( Cast<ULevel>( Outer ) )
	{
		if ( ParticleSystemComponent == NULL )
		{
			GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Emitter actor has NULL ParticleSystemComponent property - please delete!"), *GetName() ), MCACTION_DELETE, TEXT("ParticleSystemComponentNull") );
		}
	}
}


/**
 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
 * you have a component of interest but what you really want is some characteristic that you can use to track
 * down where it came from.  
 *
 */
FString AEmitter::GetDetailedInfoInternal() const
{
	FString Result;  

	if( ParticleSystemComponent != NULL )
	{
		Result = ParticleSystemComponent->GetDetailedInfoInternal();
	}
	else
	{
		Result = TEXT("No_ParticleSystemComponent");
	}

	return Result;  
}



void AEmitter::execSetTemplate(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UParticleSystem,NewTemplate);
	P_GET_UBOOL_OPTX(bDestroyOnFinish, FALSE);
	P_FINISH;

	SetTemplate(NewTemplate, bDestroyOnFinish);
}

//----------------------------------------------------------------------------

/**
 * Try to find a level color for the specified particle system component.
 */
static FColor* GetLevelColor(UParticleSystemComponent* Component)
{
	FColor* LevelColor = NULL;

	AActor* Owner = Component->GetOwner();
	if ( Owner )
	{
		ULevel* Level = Owner->GetLevel();
		ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel( Level );
		if ( LevelStreaming )
		{
			LevelColor = &LevelStreaming->DrawColor;
		}
	}

	return LevelColor;
}

/*-----------------------------------------------------------------------------
	UParticleLODLevel implementation.
-----------------------------------------------------------------------------*/
void UParticleLODLevel::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange( PropertyThatChanged );
}

void UParticleLODLevel::PostLoad()
{
	Super::PostLoad();

	ULinkerLoad* LODLinkerLoad = GetLinker();
	if (LODLinkerLoad && (LODLinkerLoad->Ver() < VER_PARTICLE_SPAWN_AND_BURST_MOVE))
	{
		ConvertToSpawnModule();
	}
	
	if (LODLinkerLoad && (LODLinkerLoad->Ver() < VER_PARTICLE_LOD_DIST_FIXUP))
	{
		// Fix-up the distributions...
		if (RequiredModule->SpawnRate.Distribution)
		{
			UObject* DArchetype = RequiredModule->SpawnRate.Distribution->GetArchetype();
			while (DArchetype && (DArchetype->IsTemplate() == FALSE))
			{
				DArchetype = DArchetype->GetArchetype();
			}

			if (DArchetype)
			{
				if (DArchetype != RequiredModule->SpawnRate.Distribution->GetArchetype())
				{
					RequiredModule->SpawnRate.Distribution->SetArchetype(DArchetype);
					MarkPackageDirty();
				}
			}
			else
			{
				warnf(TEXT("Unable to find valid archetype for RequiredModule"));
			}
		}

		if (SpawnModule)
		{
			if (SpawnModule->Rate.Distribution)
			{
				UObject* DArchetype = SpawnModule->Rate.Distribution->GetArchetype();
				while (DArchetype && (DArchetype->IsTemplate() == FALSE))
				{
					DArchetype = DArchetype->GetArchetype();
				}

				if (DArchetype)
				{
					if (DArchetype != SpawnModule->Rate.Distribution->GetArchetype())
					{
						SpawnModule->Rate.Distribution->SetArchetype(DArchetype);
						MarkPackageDirty();
					}
				}
				else
				{
					warnf(TEXT("Unable to find valid archetype for SpawnModule"));
				}
			}
		}
	}

	check(SpawnModule);
	if (LODLinkerLoad && (LODLinkerLoad->Ver() < VER_PARTICLE_BURST_LIST_ZERO))
	{
		for (INT BurstIndex = 0; BurstIndex < SpawnModule->BurstList.Num(); BurstIndex++)
		{
			if (SpawnModule->BurstList(BurstIndex).CountLow == 0)
			{
				SpawnModule->BurstList(BurstIndex).CountLow = -1;
				MarkPackageDirty();
			}
		}
	}
}

void UParticleLODLevel::UpdateModuleLists()
{
	SpawningModules.Empty();
	SpawnModules.Empty();
	UpdateModules.Empty();
	OrbitModules.Empty();
	EventReceiverModules.Empty();
	EventGenerator = NULL;

	UParticleModule* Module;
	INT TypeDataModuleIndex = -1;

	for (INT i = 0; i < Modules.Num(); i++)
	{
		Module = Modules(i);
		if (!Module)
		{
			continue;
		}

		if (Module->bSpawnModule)
		{
			SpawnModules.AddItem(Module);
		}
		if (Module->bUpdateModule)
		{
			UpdateModules.AddItem(Module);
		}

		if (Module->IsA(UParticleModuleTypeDataBase::StaticClass()))
		{
			TypeDataModule = Module;
			if (!Module->bSpawnModule && !Module->bUpdateModule)
			{
				// For now, remove it from the list and set it as the TypeDataModule
				TypeDataModuleIndex = i;
			}
		}
		else
		if (Module->IsA(UParticleModuleSpawnBase::StaticClass()))
		{
			UParticleModuleSpawnBase* SpawnBase = Cast<UParticleModuleSpawnBase>(Module);
			SpawningModules.AddItem(SpawnBase);
		}
		else
		if (Module->IsA(UParticleModuleOrbit::StaticClass()))
		{
			UParticleModuleOrbit* Orbit = Cast<UParticleModuleOrbit>(Module);
			OrbitModules.AddItem(Orbit);
		}
		else
		if (Module->IsA(UParticleModuleEventGenerator::StaticClass()))
		{
			EventGenerator = Cast<UParticleModuleEventGenerator>(Module);
		}
		else
		if (Module->IsA(UParticleModuleEventReceiverBase::StaticClass()))
		{
			UParticleModuleEventReceiverBase* Event = Cast<UParticleModuleEventReceiverBase>(Module);
			EventReceiverModules.AddItem(Event);
		}
	}

	if (EventGenerator)
	{
		// Force the event generator module to the top of the module stack...
		Modules.RemoveItem(EventGenerator);
		Modules.InsertItem(EventGenerator, 0);
	}

	if (TypeDataModuleIndex != -1)
	{
		Modules.Remove(TypeDataModuleIndex);
	}

	if (TypeDataModule /**&& (Level == 0)**/)
	{
		UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(TypeDataModule);
		if (MeshTD)
		{
			if (MeshTD->Mesh)
			{
				if (MeshTD->Mesh->LODModels(0).Elements.Num())
				{
					UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(GetOuter());
					if (SpriteEmitter && (MeshTD->bOverrideMaterial == FALSE))
					{
						FStaticMeshElement&	Element = MeshTD->Mesh->LODModels(0).Elements(0);
						if (Element.Material)
						{
							RequiredModule->Material = Element.Material;
						}
					}
				}
			}
		}
	}
}

/**
 */
UBOOL UParticleLODLevel::GenerateFromLODLevel(UParticleLODLevel* SourceLODLevel, FLOAT Percentage, UBOOL bGenerateModuleData)
{
	// See if there are already modules in place
	if (Modules.Num() > 0)
	{
		debugf(TEXT("ERROR? - GenerateFromLODLevel - modules already present!"));
		return FALSE;
	}

	UBOOL	bResult	= TRUE;

	// Allocate slots in the array...
	Modules.InsertZeroed(0, SourceLODLevel->Modules.Num());

	// Set the enabled flag
	bEnabled = SourceLODLevel->bEnabled;

	// Required module...
	RequiredModule = CastChecked<UParticleModuleRequired>(
		SourceLODLevel->RequiredModule->GenerateLODModule(SourceLODLevel, this, Percentage, bGenerateModuleData));

	// Spawn module...
	SpawnModule = CastChecked<UParticleModuleSpawn>(
		SourceLODLevel->SpawnModule->GenerateLODModule(SourceLODLevel, this, Percentage, bGenerateModuleData));

	// TypeData module, if present...
	if (SourceLODLevel->TypeDataModule)
	{
		TypeDataModule = SourceLODLevel->TypeDataModule->GenerateLODModule(SourceLODLevel, this, Percentage, bGenerateModuleData);
	}

	// The remaining modules...
	for (INT ModuleIndex = 0; ModuleIndex < SourceLODLevel->Modules.Num(); ModuleIndex++)
	{
		if (SourceLODLevel->Modules(ModuleIndex))
		{
			Modules(ModuleIndex) = SourceLODLevel->Modules(ModuleIndex)->GenerateLODModule(SourceLODLevel, this, Percentage, bGenerateModuleData);
		}
		else
		{
			Modules(ModuleIndex) = NULL;
		}
	}

	return bResult;
}

/**
 *	CalculateMaxActiveParticleCount
 *	Determine the maximum active particles that could occur with this emitter.
 *	This is to avoid reallocation during the life of the emitter.
 *
 *	@return		The maximum active particle count for the LOD level.
 */
INT	UParticleLODLevel::CalculateMaxActiveParticleCount()
{
	check(RequiredModule != NULL);

	// Determine the lifetime for particles coming from the emitter
	FLOAT ParticleLifetime = 0.0f;
	UBOOL bHadZerorParticleLifetime = FALSE;

	TArray<UParticleModuleLifetimeBase*> LifetimeModules;
	LifetimeModules.Empty();
	for (INT ModuleIndex = 0; ModuleIndex < Modules.Num(); ModuleIndex++)
	{
		UParticleModule* Module = Modules(ModuleIndex);
		if (Module)
		{
			UParticleModuleLifetime* LifetimeMod = Cast<UParticleModuleLifetime>(Module);
			if (LifetimeMod)
			{
				LifetimeModules.AddItem(LifetimeMod);
			}
		}
	}

	if (LifetimeModules.Num() > 0)
	{
		for (INT LTMIndex = 0; LTMIndex < LifetimeModules.Num(); LTMIndex++)
		{
			UParticleModuleLifetimeBase* LifetimeBase = LifetimeModules(LTMIndex);
			FLOAT PartLife = LifetimeBase->GetMaxLifetime();

			if (PartLife == 0.0f)
			{
				bHadZerorParticleLifetime = TRUE;
			}
			ParticleLifetime += PartLife;
		}
	}
	else
	{
		warnf(TEXT("LODLevel has no Lifetime module - PSys %s"), *GetOuter()->GetName());
	}

	// Check for an UberRaiDrops module
	for (INT ModuleIndex = 0; ModuleIndex < Modules.Num(); ModuleIndex++)
	{
		UParticleModule* Module = Modules(ModuleIndex);
		if (Module)
		{
			UParticleModuleUberRainDrops* URDMod = Cast<UParticleModuleUberRainDrops>(Module);
			if (URDMod)
			{
				ParticleLifetime += URDMod->LifetimeMax;
			}
		}
	}

	// Calculate the Spawn distribution contribution
	FLOAT MaxSpawnRate, MinSpawnRate;
	SpawnModule->Rate.GetOutRange(MinSpawnRate, MaxSpawnRate);

	// Calculate the BurstList contribution
	INT MaxBurst = 0;
	for (INT BurstIndex = 0; BurstIndex < SpawnModule->BurstList.Num(); BurstIndex++)
	{
		FParticleBurst& BurstEntry = SpawnModule->BurstList(BurstIndex);
		//@todo. Take time into account??
		MaxBurst += BurstEntry.Count;
	}

	// Determine the max
	INT MaxAPC = 0;

	MaxAPC += appFloor(MaxSpawnRate * ParticleLifetime) + 2;
	MaxAPC += MaxBurst;

	PeakActiveParticles = MaxAPC;

	return MaxAPC;
}

/**
 *	Update to the new SpawnModule method
 */
void UParticleLODLevel::ConvertToSpawnModule()
{
	// Move the required module SpawnRate and Burst information to a new SpawnModule.
	if (SpawnModule)
	{
//		warnf(TEXT("LOD Level already has a spawn module!"));
		return;
	}

	UParticleEmitter* EmitterOuter = CastChecked<UParticleEmitter>(GetOuter());
	SpawnModule = ConstructObject<UParticleModuleSpawn>(UParticleModuleSpawn::StaticClass(), EmitterOuter->GetOuter());
	check(SpawnModule);

	// First the SpawnRate...
	SpawnModule->Rate = RequiredModule->SpawnRate;

	UDistributionFloat* SourceDist = RequiredModule->SpawnRate.Distribution;
	if (SourceDist)
	{
		SpawnModule->Rate.Distribution = Cast<UDistributionFloat>(UObject::StaticDuplicateObject(SourceDist, SourceDist, SpawnModule, TEXT("None")));
		SpawnModule->Rate.Distribution->bIsDirty = TRUE;
	}

	// Now the burst list.
	INT BurstCount = RequiredModule->BurstList.Num();
	if (BurstCount > 0)
	{
		SpawnModule->BurstList.AddZeroed(BurstCount);
		for (INT BurstIndex = 0; BurstIndex < BurstCount; BurstIndex++)
		{
			SpawnModule->BurstList(BurstIndex).Count = RequiredModule->BurstList(BurstIndex).Count;
			SpawnModule->BurstList(BurstIndex).CountLow = RequiredModule->BurstList(BurstIndex).CountLow;
			SpawnModule->BurstList(BurstIndex).Time = RequiredModule->BurstList(BurstIndex).Time;
		}
	}

	MarkPackageDirty();
}

/**
 *	Return the index of the given module if it is contained in the LOD level
 */
INT UParticleLODLevel::GetModuleIndex(UParticleModule* InModule)
{
	if (InModule)
	{
		if (InModule == RequiredModule)
		{
			return INDEX_REQUIREDMODULE;
		}
		else if (InModule == SpawnModule)
		{
			return INDEX_SPAWNMODULE;
		}
		else if (InModule == TypeDataModule)
		{
			return INDEX_TYPEDATAMODULE;
		}
		else
		{
			for (INT ModuleIndex = 0; ModuleIndex < Modules.Num(); ModuleIndex++)
			{
				if (InModule == Modules(ModuleIndex))
				{
					return ModuleIndex;
				}
			}
		}
	}

	return INDEX_NONE;
}

/**
 *	Return the module at the given index if it is contained in the LOD level
 */
UParticleModule* UParticleLODLevel::GetModuleAtIndex(INT InIndex)
{
	// 'Normal' modules
	if (InIndex > INDEX_NONE)
	{
		if (InIndex < Modules.Num())
		{
			return Modules(InIndex);
		}

		return NULL;
	}

	switch (InIndex)
	{
	case INDEX_REQUIREDMODULE:		return RequiredModule;
	case INDEX_SPAWNMODULE:			return SpawnModule;
	case INDEX_TYPEDATAMODULE:		return TypeDataModule;
	}

	return NULL;
}

/**
 *	Sets the LOD 'Level' to the given value, properly updating the modules LOD validity settings.
 *	This function assumes that any error-checking of values was done by the caller!
 *	It also assumes that when inserting an LOD level, indices will be shifted from lowest to highest...
 *	When removing one, the will go from highest to lowest.
 */
void UParticleLODLevel::SetLevelIndex(INT InLevelIndex)
{
	// Remove the 'current' index from the validity flags and set the new one.
	RequiredModule->LODValidity &= ~(1 << Level);
	RequiredModule->LODValidity |= (1 << InLevelIndex);
	SpawnModule->LODValidity &= ~(1 << Level);
	SpawnModule->LODValidity |= (1 << InLevelIndex);
	if (TypeDataModule)
	{
		TypeDataModule->LODValidity &= ~(1 << Level);
		TypeDataModule->LODValidity |= (1 << InLevelIndex);
	}
	for (INT ModuleIndex = 0; ModuleIndex < Modules.Num(); ModuleIndex++)
	{
		UParticleModule* CheckModule = Modules(ModuleIndex);
		if (CheckModule)
		{
			CheckModule->LODValidity &= ~(1 << Level);
			CheckModule->LODValidity |= (1 << InLevelIndex);
		}
	}

	Level = InLevelIndex;
}

/**
 *	Return TRUE if the given module is editable for this LOD level.
 *	
 *	@param	InModule	The module of interest.
 *	@return	TRUE		If it is editable for this LOD level.
 *			FALSE		If it is not.
 */
UBOOL UParticleLODLevel::IsModuleEditable(UParticleModule* InModule)
{
	// If the module validity flag is not set for this level, it is not editable.
	if ((InModule->LODValidity & (1 << Level)) == 0)
	{
		return FALSE;
	}

	// If the module is shared w/ higher LOD levels, then it is not editable...
	INT Validity = 0;
	if (Level > 0)
	{
		INT Check = Level - 1;
		while (Check >= 0)
		{
			Validity |= (1 << Check);
			Check--;
		}

		if ((Validity & InModule->LODValidity) != 0)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/*-----------------------------------------------------------------------------
	UParticleEmitter implementation.
-----------------------------------------------------------------------------*/
FParticleEmitterInstance* UParticleEmitter::CreateInstance(UParticleSystemComponent* InComponent)
{
	appErrorf(TEXT("UParticleEmitter::CreateInstance is pure virtual")); 
	return NULL; 
}

void UParticleEmitter::UpdateModuleLists()
{
	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels(LODIndex);
		if (LODLevel)
		{
			LODLevel->UpdateModuleLists();
		}
	}
}

/**
 *	Helper function for fixing up LODValidity issues on particle modules...
 *
 *	@param	LODIndex		The index of the LODLevel the module is from.
 *	@param	ModuleIndex		The index of the module being checked.
 *	@param	Emitter			The emitter owner.
 *	@param	CurrModule		The module being checked.
 *
 *	@return	 0		If there was no problem.
 *			 1		If there was a problem and it was fixed.
 *			-1		If there was a problem that couldn't be fixed.
 */
INT ParticleEmitterHelper_FixupModuleLODErrors( INT LODIndex, INT ModuleIndex, 
	const UParticleEmitter* Emitter, UParticleModule* CurrModule )
{
	INT Result = 1;
	UBOOL bIsDirty = FALSE;

	UObject* ModuleOuter = CurrModule->GetOuter();
	UObject* EmitterOuter = Emitter->GetOuter();
	if (ModuleOuter != EmitterOuter)
	{
		// Module has an incorrect outer
		CurrModule->Rename(NULL, EmitterOuter, REN_ForceNoResetLoaders|REN_DoNotDirty);
		bIsDirty = TRUE;
	}

	if (CurrModule->LODValidity == 0)
	{
		// Immediately tag it for this lod level...
		CurrModule->LODValidity = (1 << LODIndex);
		bIsDirty = TRUE;
	}
	else
	if (CurrModule->IsUsedInLODLevel(LODIndex) == FALSE)
	{
		// Why was this even called here?? 
		// The assumption is that it should be called for the module in the given lod level...
		// So, we will tag it with this index.
		CurrModule->LODValidity |= (1 << LODIndex);
		bIsDirty = TRUE;
	}

	if (LODIndex > 0)
	{
		INT CheckIndex = LODIndex - 1;
		while (CheckIndex >= 0)
		{
			if (CurrModule->IsUsedInLODLevel(CheckIndex))
			{
				// Ensure that it is the same as the one it THINKS it is shared with...
				UParticleLODLevel* CheckLODLevel = Emitter->LODLevels(CheckIndex);

				if (CurrModule->IsA(UParticleModuleSpawn::StaticClass()))
				{
					if (CheckLODLevel->SpawnModule != CurrModule)
					{
						// Fix it up... Turn off the higher LOD flag
						CurrModule->LODValidity &= ~(1 << CheckIndex);
						bIsDirty = TRUE;
					}
				}
				else
				if (CurrModule->IsA(UParticleModuleRequired::StaticClass()))
				{
					if (CheckLODLevel->RequiredModule != CurrModule)
					{
						// Fix it up... Turn off the higher LOD flag
						CurrModule->LODValidity &= ~(1 << CheckIndex);
						bIsDirty = TRUE;
					}
				}
				else
				if (CurrModule->IsA(UParticleModuleTypeDataBase::StaticClass()))
				{
					if (CheckLODLevel->TypeDataModule != CurrModule)
					{
						// Fix it up... Turn off the higher LOD flag
						CurrModule->LODValidity &= ~(1 << CheckIndex);
						bIsDirty = TRUE;
					}
				}
				else
				{
					if (ModuleIndex >= CheckLODLevel->Modules.Num())
					{
						warnf(NAME_Log, TEXT("\t\tMismatched module count at %2d in %s"), LODIndex, *(Emitter->GetPathName()));
						Result = -1;
					}
					else
					{
						UParticleModule* CheckModule = CheckLODLevel->Modules(ModuleIndex);
						if (CheckModule != CurrModule)
						{
							// Fix it up... Turn off the higher LOD flag
							CurrModule->LODValidity &= ~(1 << CheckIndex);
							bIsDirty = TRUE;
						}
					}
				}
			}

			CheckIndex--;
		}
	}

	if ((bIsDirty == TRUE) && GIsUCC)
	{
		CurrModule->MarkPackageDirty();
		Emitter->MarkPackageDirty();
	}

	return Result;
}

void UParticleEmitter::PostLoad()
{
	Super::PostLoad();

	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels(LODIndex);
		if (LODLevel)
		{
			LODLevel->ConditionalPostLoad();

			ULinkerLoad* LODLevelLinker = LODLevel->GetLinker();
			if (LODLevel->SpawnModule == NULL)
			{
				// This indicates that the LOD level was not PostLoaded correctly...
				if (LODLevelLinker && (LODLevelLinker->Ver() < VER_PARTICLE_SPAWN_AND_BURST_MOVE))
				{
					warnf(TEXT("Version number indicates SpawnModule conversion should have taken place!"));
				}
				// Force the conversion to SpawnModule
				UParticleSystem* PSys = Cast<UParticleSystem>(GetOuter());
				if (PSys)
				{
					warnf(TEXT("LODLevel %d was not converted to spawn module - forcing: %s"), 
						LODLevel->Level, *(PSys->GetPathName()));
				}
				LODLevel->ConvertToSpawnModule();
			}

			check(LODLevel->SpawnModule);
			if (LODLevelLinker && (LODLevelLinker->Ver() < VER_PARTICLE_BURST_LIST_ZERO))
			{
				for (INT BurstIndex = 0; BurstIndex < LODLevel->SpawnModule->BurstList.Num(); BurstIndex++)
				{
					if (LODLevel->SpawnModule->BurstList(BurstIndex).CountLow == 0)
					{
//						warnf(TEXT("Version number indicates BurstList conversion should have taken place!"));
						LODLevel->SpawnModule->BurstList(BurstIndex).CountLow = -1;
						LODLevel->MarkPackageDirty();
					}
				}
			}

			if (LODLevelLinker && (LODLevelLinker->Ver() < VER_MODSHADOWMESHPIXELSHADER_ATTENALLOWED) && (LODIndex > 0))
			{
				UParticleLODLevel* LODLevelZero = LODLevels(0);
				if (LODLevelZero)
				{
					if (LODLevelZero->TypeDataModule)
					{
						if ((LODLevel->TypeDataModule == NULL) || 
							(LODLevel->TypeDataModule->GetClass() != LODLevelZero->TypeDataModule->GetClass()))
						{
							UParticleSystem* PSys = Cast<UParticleSystem>(GetOuter());
							if (PSys)
							{
								warnf(TEXT("LODLevel %d had an invalid type data module - fixing: %s"), LODLevel->Level, *(PSys->GetPathName()));
							}
							LODLevel->TypeDataModule = LODLevelZero->TypeDataModule->GenerateLODModule(LODLevelZero, LODLevel, 100.0f, FALSE);
							LODLevel->MarkPackageDirty();
						}
					}
					else
					{
						if (LODLevel->TypeDataModule != NULL)
						{
							UParticleSystem* PSys = Cast<UParticleSystem>(GetOuter());
							if (PSys)
							{
								warnf(TEXT("LODLevel %d had an invalid type data module - fixing: %s"), LODLevel->Level, *(PSys->GetPathName()));
							}
							LODLevel->TypeDataModule = NULL;
							LODLevel->MarkPackageDirty();
						}
					}
				}
			}
		}
	}

	if (GIsUCCMake == TRUE)
	{
		return;
	}

	if ((GIsEditor == TRUE) && 1)//(GIsUCC == FALSE))
	{
		ConvertedModules = FALSE;
		PeakActiveParticles = 0;

		// Check for improper outers...
		UObject* EmitterOuter = GetOuter();
		UBOOL bWarned = FALSE;
		for (INT LODIndex = 0; (LODIndex < LODLevels.Num()) && !bWarned; LODIndex++)
		{
			UParticleLODLevel* LODLevel = LODLevels(LODIndex);
			if (LODLevel)
			{
				LODLevel->ConditionalPostLoad();

				UParticleModule* Module = LODLevel->TypeDataModule;
				if (Module)
				{
					Module->ConditionalPostLoad();

					UObject* OuterObj = Module->GetOuter();
					check(OuterObj);
					if (OuterObj != EmitterOuter)
					{
						warnf(TEXT("UParticleModule %s has an incorrect outer on %s... run FixupEmitters on package %s (%s)"),
							*(Module->GetPathName()), 
							*(EmitterOuter->GetPathName()),
							*(OuterObj->GetOutermost()->GetPathName()),
							*(GetOutermost()->GetPathName()));
						warnf(TEXT("\tModule Outer..............%s"), *(OuterObj->GetPathName()));
						warnf(TEXT("\tModule Outermost..........%s"), *(Module->GetOutermost()->GetPathName()));
						warnf(TEXT("\tEmitter Outer.............%s"), *(EmitterOuter->GetPathName()));
						warnf(TEXT("\tEmitter Outermost.........%s"), *(GetOutermost()->GetPathName()));
						bWarned = TRUE;
					}
				}

				if (!bWarned)
				{
					for (INT ModuleIndex = 0; (ModuleIndex < LODLevel->Modules.Num()) && !bWarned; ModuleIndex++)
					{
						Module = LODLevel->Modules(ModuleIndex);
						if (Module)
						{
							Module->ConditionalPostLoad();

							UObject* OuterObj = Module->GetOuter();
							check(OuterObj);
							if (OuterObj != EmitterOuter)
							{
								warnf(TEXT("UParticleModule %s has an incorrect outer on %s... run FixupEmitters on package %s (%s)"),
									*(Module->GetPathName()), 
									*(EmitterOuter->GetPathName()),
									*(OuterObj->GetOutermost()->GetPathName()),
									*(GetOutermost()->GetPathName()));
								warnf(TEXT("\tModule Outer..............%s"), *(OuterObj->GetPathName()));
								warnf(TEXT("\tModule Outermost..........%s"), *(Module->GetOutermost()->GetPathName()));
								warnf(TEXT("\tEmitter Outer.............%s"), *(EmitterOuter->GetPathName()));
								warnf(TEXT("\tEmitter Outermost.........%s"), *(GetOutermost()->GetPathName()));
								bWarned = TRUE;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
		{
			UParticleLODLevel* LODLevel = LODLevels(LODIndex);
			if (LODLevel)
			{
				LODLevel->ConditionalPostLoad();
			}
		}
	}
   
	if (GetLinker() && (GetLinker()->Ver() < VER_PARTICLE_LOD_MODULE_SHARE))
	{
		if (IsTemplate() == FALSE)
		{
			check(LODLevels.Num() >= 2);
			if (LODLevels.Num() == 2)
			{
				// Remove duplicate modules across LOD levels where possible...
				INT HighLODIndex = 0;
				UParticleLODLevel* HighLOD = LODLevels(HighLODIndex);
				if (HighLOD->IsTemplate() == FALSE)
				{
					// Update the bEnabled flag for the high LOD level...
					check(HighLOD->RequiredModule);
					HighLOD->bEnabled = HighLOD->RequiredModule->bEnabled;

					for (INT LODIndex = 1; LODIndex < LODLevels.Num(); LODIndex++)
					{
						UParticleLODLevel* CurrLOD = LODLevels(LODIndex);
						check(CurrLOD);

						UParticleModule* HighModule;
						UParticleModule* CurrModule;

						// Type data
						if (HighLOD->TypeDataModule)
						{
							HighModule = HighLOD->TypeDataModule;
							CurrModule = CurrLOD->TypeDataModule;

							HighModule->LODValidity |= (1 << HighLODIndex);
							if (HighModule->IsIdentical(CurrModule) || (CurrModule == NULL))
							{
								HighModule->LODValidity |= (1 << LODIndex);
								CurrLOD->TypeDataModule = HighLOD->TypeDataModule;
							}
							else
							{
								CurrModule->LODValidity |= (1 << LODIndex);
							}
						}

						// Required
						check(HighLOD->RequiredModule);
						check(CurrLOD->RequiredModule);

						HighModule = HighLOD->RequiredModule;
						CurrModule = CurrLOD->RequiredModule;

						HighModule->LODValidity |= (1 << HighLODIndex);

						// The required module enabled flag WAS being used for LOD enabled...
						CurrLOD->bEnabled = CurrModule->bEnabled;

						// Take the bEnabled out of the comparison... but only for Required modules!
						CurrModule->bEnabled = HighModule->bEnabled;
						if (HighModule->IsIdentical(CurrModule))
						{
							HighModule->LODValidity |= (1 << LODIndex);
							CurrLOD->RequiredModule = HighLOD->RequiredModule;
						}
						else
						{
							CurrModule->LODValidity |= (1 << LODIndex);
						}

						// Spawn
						check(HighLOD->SpawnModule);
						check(CurrLOD->SpawnModule);

						HighModule = HighLOD->SpawnModule;
						CurrModule = CurrLOD->SpawnModule;

						HighModule->LODValidity |= (1 << HighLODIndex);
						if (HighModule->IsIdentical(CurrModule))
						{
							HighModule->LODValidity |= (1 << LODIndex);
							CurrLOD->SpawnModule = HighLOD->SpawnModule;
						}
						else
						{
							CurrModule->LODValidity |= (1 << LODIndex);
						}

						// Remaining modules
						for (INT ModuleIndex = 0; ModuleIndex < HighLOD->Modules.Num(); ModuleIndex++)
						{
							HighModule = HighLOD->Modules(ModuleIndex);
							CurrModule = CurrLOD->Modules(ModuleIndex);

							HighModule->LODValidity |= (1 << HighLODIndex);
							if (HighModule->IsIdentical(CurrModule))
							{
								HighModule->LODValidity |= (1 << LODIndex);
								CurrLOD->Modules(ModuleIndex) = HighModule;
							}
							else
							{
								CurrModule->LODValidity |= (1 << LODIndex);
							}
						}

						HighLOD = CurrLOD;
						HighLODIndex = LODIndex;
					}
				}
			}
			else
			{
				for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
				{
					UParticleLODLevel* CurrLOD = LODLevels(LODIndex);
					check(CurrLOD);

					UParticleModule* CurrModule;

					// Type data
					if (CurrLOD->TypeDataModule)
					{
						CurrModule = CurrLOD->TypeDataModule;
						CurrModule->LODValidity |= (1 << LODIndex);
					}

					// Spawn
					if (CurrLOD->SpawnModule)
					{
						CurrModule = CurrLOD->SpawnModule;
						CurrModule->LODValidity |= (1 << LODIndex);
					}

					// Remaining modules
					for (INT ModuleIndex = 0; ModuleIndex < CurrLOD->Modules.Num(); ModuleIndex++)
					{
						CurrModule = CurrLOD->Modules(ModuleIndex);
						CurrModule->LODValidity |= (1 << LODIndex);
					}
				}
			}
		}
	}

	// Fix up the TypeData modules...
	if (GetLinker() && (GetLinker()->Ver() < VER_PARTICLE_LOD_MODULE_TYPEDATA_FIXUP))
	{
		if (IsTemplate() == FALSE)
		{
			// Remove duplicate modules across LOD levels where possible...
			INT HighLODIndex = 0;
			UParticleLODLevel* HighLOD = LODLevels(HighLODIndex);
			if (HighLOD->IsTemplate() == FALSE)
			{
				// Update the bEnabled flag for the high LOD level...
				check(HighLOD->RequiredModule);
				for (INT LODIndex = 1; LODIndex < LODLevels.Num(); LODIndex++)
				{
					UParticleLODLevel* CurrLOD = LODLevels(LODIndex);
					check(CurrLOD);

					UParticleModule* HighModule;
					UParticleModule* CurrModule;

					// Type data
					if (HighLOD->TypeDataModule)
					{
						HighModule = HighLOD->TypeDataModule;
						CurrModule = CurrLOD->TypeDataModule;

						HighModule->LODValidity |= (1 << HighLODIndex);
						if (HighModule->IsIdentical(CurrModule))
						{
							HighModule->LODValidity |= (1 << LODIndex);
							CurrLOD->TypeDataModule = HighLOD->TypeDataModule;
						}
						else if (CurrModule == NULL)
						{
							HighModule->LODValidity |= (1 << LODIndex);
							CurrLOD->TypeDataModule = HighLOD->TypeDataModule;

							// Have to resave!
							MarkPackageDirty(TRUE);
						}
						else
						{
							CurrModule->LODValidity |= (1 << LODIndex);
						}
					}

					HighLOD = CurrLOD;
					HighLODIndex = LODIndex;
				}
			}
		}
	}

	if ((GIsEditor || GIsUCC) && (GetLinker() && (GetLinker()->Ver() < VER_EMITTER_LODVALIDITY_FIX2)))
	{
		// Fix up possible '0' validity modules...
		// Fix up invalid LODValidity flags...
		if (IsTemplate() == FALSE)
		{
			// For each LOD level, check the validity flags on the modules and
			// verify 'shared' modules are really shared...
			UParticleLODLevel* PrevLOD = NULL;
			UParticleLODLevel* CurrLOD = NULL;

			for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
			{
				CurrLOD = LODLevels(LODIndex);
				if (CurrLOD == NULL)
				{
					continue;
				}

				// Check the spawn module
				check(CurrLOD->SpawnModule);
				ParticleEmitterHelper_FixupModuleLODErrors(LODIndex, INDEX_SPAWNMODULE, this, CurrLOD->SpawnModule);

				// Check the required module
				check(CurrLOD->RequiredModule);
				ParticleEmitterHelper_FixupModuleLODErrors(LODIndex, INDEX_REQUIREDMODULE, this, CurrLOD->RequiredModule);

				// Check the type data module (if present)
				if (CurrLOD->TypeDataModule)
				{
					if (PrevLOD)
					{
						if (PrevLOD->TypeDataModule == NULL)
						{
							warnf(NAME_Log, TEXT("\tEmitter: Missing TDM in LOD %2d: %s"), PrevLOD->Level, *(GetPathName()));
						}

						if (PrevLOD->TypeDataModule->IsA(CurrLOD->TypeDataModule->GetClass()) == FALSE)
						{
							warnf(NAME_Log, TEXT("\tEmitter: Mismatched TDM in LOD %2d: %s"), PrevLOD->Level, *(GetPathName()));
						}
					}

					ParticleEmitterHelper_FixupModuleLODErrors(LODIndex, INDEX_TYPEDATAMODULE, this, CurrLOD->TypeDataModule);
				}
				else if (PrevLOD && PrevLOD->TypeDataModule)
				{
					warnf(NAME_Log, TEXT("\tEmitter: Missing TDM in LOD %2d: %s"), LODIndex, *(GetPathName()));
				}

				if (PrevLOD)
				{
					if (PrevLOD->Modules.Num() != CurrLOD->Modules.Num())
					{
						warnf(NAME_Log, TEXT("\tEmitter: Mismatched modules counts in LOD %2d: %s"), LODIndex, *(GetPathName()));
					}
				}

				// Check the remaining modules
				for (INT ModuleIndex = 0; ModuleIndex < CurrLOD->Modules.Num(); ModuleIndex++)
				{
					UParticleModule* CurrModule = CurrLOD->Modules(ModuleIndex);
					if (PrevLOD)
					{
						if (PrevLOD->Modules.Num() <= ModuleIndex)
						{
							warnf(NAME_Log, TEXT("\tEmitter: Missing module %2d in LOD %2d: %s"), ModuleIndex, PrevLOD->Level, *(GetPathName()));
						}
					}
					ParticleEmitterHelper_FixupModuleLODErrors(LODIndex, ModuleIndex, this, CurrModule);
				}

				PrevLOD = CurrLOD;
			}
		}
	}

	ConvertedModules = TRUE;

	// this will look at all of the emitters and then remove ones that some how have become NULL (e.g. from a removal of an Emitter where content
	// is still referencing it)
	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels(LODIndex);
		if (LODLevel)
		{
			for (INT ModuleIndex = LODLevel->Modules.Num()-1; ModuleIndex >= 0; ModuleIndex--)
			{
				UParticleModule* ParticleModule = LODLevel->Modules(ModuleIndex);
				if( ParticleModule == NULL )
				{
					LODLevel->Modules.Remove(ModuleIndex);
					MarkPackageDirty( TRUE );
				}
			}
		}
	}


	UObject* MyOuter = GetOuter();
	UParticleSystem* PSysOuter = Cast<UParticleSystem>(MyOuter);
	UBOOL bRegenDup = FALSE;
	if (PSysOuter)
	{
		bRegenDup = PSysOuter->bRegenerateLODDuplicate;
	}

	UpdateModuleLists();
}

void UParticleEmitter::PreEditChange(UProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);
}

void UParticleEmitter::PostEditChange(UProperty* PropertyThatChanged)
{
	check(GIsEditor);

	// Reset the peak active particle counts.
	// This could check for changes to SpawnRate and Burst and only reset then,
	// but since we reset the particle system after any edited property, it
	// may as well just autoreset the peak counts.
	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels(LODIndex);
		if (LODLevel)
		{
			LODLevel->PeakActiveParticles	= 1;
		}
	}

	UpdateModuleLists();

	for (TObjectIterator<UParticleSystemComponent> It;It;++It)
	{
		if (It->Template)
		{
			INT i;

			for (i=0; i<It->Template->Emitters.Num(); i++)
			{
				if (It->Template->Emitters(i) == this)
				{
					It->UpdateInstances();
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);

	if (CalculateMaxActiveParticleCount() == FALSE)
	{
		//
	}
}

void UParticleEmitter::SetEmitterName(FName Name)
{
	EmitterName = Name;
}

FName& UParticleEmitter::GetEmitterName()
{
	return EmitterName;
}

void UParticleEmitter::SetLODCount(INT LODCount)
{
	// 
}

void UParticleEmitter::AddEmitterCurvesToEditor(UInterpCurveEdSetup* EdSetup)
{
	debugf(TEXT("UParticleEmitter::AddEmitterCurvesToEditor> Should no longer be called..."));
	return;
}

void UParticleEmitter::RemoveEmitterCurvesFromEditor(UInterpCurveEdSetup* EdSetup)
{
	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels(LODIndex);
		// Remove the typedata curves...
		if (LODLevel->TypeDataModule && LODLevel->TypeDataModule->IsDisplayedInCurveEd(EdSetup))
		{
			LODLevel->TypeDataModule->RemoveModuleCurvesFromEditor(EdSetup);
		}

		// Remove the spawn module curves...
		if (LODLevel->SpawnModule && LODLevel->SpawnModule->IsDisplayedInCurveEd(EdSetup))
		{
			LODLevel->SpawnModule->RemoveModuleCurvesFromEditor(EdSetup);
		}

		// Remove each modules curves as well.
		for (INT ii = 0; ii < LODLevel->Modules.Num(); ii++)
		{
			if (LODLevel->Modules(ii)->IsDisplayedInCurveEd(EdSetup))
			{
				// Remove it from the curve editor!
				LODLevel->Modules(ii)->RemoveModuleCurvesFromEditor(EdSetup);
			}
		}
	}
}

void UParticleEmitter::ChangeEditorColor(FColor& Color, UInterpCurveEdSetup* EdSetup)
{
	UParticleLODLevel* LODLevel = LODLevels(0);
	LODLevel->RequiredModule->EmitterEditorColor	= Color;
	for (INT TabIndex = 0; TabIndex < EdSetup->Tabs.Num(); TabIndex++)
	{
		FCurveEdTab*	Tab = &(EdSetup->Tabs(TabIndex));
		for (INT CurveIndex = 0; CurveIndex < Tab->Curves.Num(); CurveIndex++)
		{
			FCurveEdEntry* Entry	= &(Tab->Curves(CurveIndex));
			if (LODLevel->SpawnModule->Rate.Distribution == Entry->CurveObject)
			{
				Entry->CurveColor	= Color;
			}
		}
	}
}

void UParticleEmitter::AutoPopulateInstanceProperties(UParticleSystemComponent* PSysComp)
{
	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel	= LODLevels(LODIndex);
		for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
		{
			UParticleModule* Module = LODLevel->Modules(ModuleIndex);
			Module->AutoPopulateInstanceProperties(PSysComp);
		}
	}
}

/** CreateLODLevel
 *	Creates the given LOD level.
 *	Intended for editor-time usage.
 *	Assumes that the given LODLevel will be in the [0..100] range.
 *	
 *	@return The index of the created LOD level
 */
INT UParticleEmitter::CreateLODLevel(INT LODLevel, UBOOL bGenerateModuleData)
{
	INT					LevelIndex		= -1;
	UParticleLODLevel*	CreatedLODLevel	= NULL;

	if (LODLevels.Num() == 0)
	{
		LODLevel = 0;
	}

	// Is the requested index outside a viable range?
	if ((LODLevel < 0) || (LODLevel > LODLevels.Num()))
	{
		return -1;
	}

	// NextHighestLODLevel is the one that will be 'copied'
	UParticleLODLevel*	NextHighestLODLevel	= NULL;
	INT NextHighIndex = -1;
	// NextLowestLODLevel is the one (and all ones lower than it) that will have their LOD indices updated
	UParticleLODLevel*	NextLowestLODLevel	= NULL;
	INT NextLowIndex = -1;

	// Grab the two surrounding LOD levels...
	if (LODLevel == 0)
	{
		// It is being added at the front of the list... (highest)
		if (LODLevels.Num() > 0)
		{
			NextHighestLODLevel = LODLevels(0);
			NextHighIndex = 0;
			NextLowestLODLevel = NextHighestLODLevel;
			NextLowIndex = 0;
		}
	}
	else
	if (LODLevel > 0)
	{
		NextHighestLODLevel = LODLevels(LODLevel - 1);
		NextHighIndex = LODLevel - 1;
		if (LODLevel < LODLevels.Num())
		{
			NextLowestLODLevel = LODLevels(LODLevel);
			NextLowIndex = LODLevel;
		}
	}

	// Update the LODLevel index for the lower levels and
	// offset the LOD validity flags for the modules...
	if (NextLowestLODLevel)
	{
		for (INT LowIndex = LODLevels.Num() - 1; LowIndex >= NextLowIndex; LowIndex--)
		{
			UParticleLODLevel* LowRemapLevel = LODLevels(LowIndex);
			if (LowRemapLevel)
			{
				LowRemapLevel->SetLevelIndex(LowIndex + 1);
			}
		}
	}

	// Create a ParticleLODLevel
	CreatedLODLevel = ConstructObject<UParticleLODLevel>(UParticleLODLevel::StaticClass(), this);
	check(CreatedLODLevel);

	CreatedLODLevel->Level = LODLevel;
	CreatedLODLevel->ConvertedModules = TRUE;
	CreatedLODLevel->PeakActiveParticles = 0;

	// Determine where to place it...
	if (LODLevels.Num() == 0)
	{
		LODLevels.InsertZeroed(0, 1);
		LODLevels(0) = CreatedLODLevel;
		CreatedLODLevel->Level	= 0;
	}
	else
	{
		LODLevels.InsertZeroed(LODLevel, 1);
		LODLevels(LODLevel) = CreatedLODLevel;
		CreatedLODLevel->Level = LODLevel;
	}

	if (NextHighestLODLevel)
	{
		// Generate from the higher LOD level
		if (CreatedLODLevel->GenerateFromLODLevel(NextHighestLODLevel, 100.0, bGenerateModuleData) == FALSE)
		{
			warnf(TEXT("Failed to generate LOD level %d from level %d"), LODLevel, NextHighestLODLevel->Level);
		}
	}
	else
	{
		// Create the RequiredModule
		UParticleModuleRequired* RequiredModule	= ConstructObject<UParticleModuleRequired>(UParticleModuleRequired::StaticClass(), GetOuter());
		check(RequiredModule);
		CreatedLODLevel->RequiredModule	= RequiredModule;

		// The SpawnRate for the required module
		RequiredModule->bUseLocalSpace			= FALSE;
		RequiredModule->bKillOnDeactivate		= FALSE;
		RequiredModule->bKillOnCompleted		= FALSE;
		RequiredModule->EmitterDuration			= 1.0f;
		RequiredModule->EmitterLoops			= 0;
		RequiredModule->ParticleBurstMethod		= EPBM_Instant;
		RequiredModule->ModuleEditorColor		= FColor::MakeRandomColor();
		RequiredModule->InterpolationMethod		= PSUVIM_None;
		RequiredModule->SubImages_Horizontal	= 1;
		RequiredModule->SubImages_Vertical		= 1;
		RequiredModule->bScaleUV				= FALSE;
		RequiredModule->RandomImageTime			= 0.0f;
		RequiredModule->RandomImageChanges		= 0;
		RequiredModule->bDirectUV				= FALSE;
		RequiredModule->EmitterRenderMode		= ERM_Normal;
		RequiredModule->EmitterEditorColor		= FColor::MakeRandomColor();
		RequiredModule->bEnabled				= TRUE;

		RequiredModule->LODValidity = 1;

		// There must be a spawn module as well...
		UParticleModuleSpawn* SpawnModule = ConstructObject<UParticleModuleSpawn>(UParticleModuleSpawn::StaticClass(), GetOuter());
		check(SpawnModule);
		CreatedLODLevel->SpawnModule = SpawnModule;
		SpawnModule->LODValidity = 1;
		UDistributionFloatConstant* ConstantSpawn	= Cast<UDistributionFloatConstant>(SpawnModule->Rate.Distribution);
		ConstantSpawn->Constant					= 10;
		ConstantSpawn->bIsDirty					= TRUE;
		SpawnModule->BurstList.Empty();

		// Copy the TypeData module
		CreatedLODLevel->TypeDataModule			= NULL;
	}

	LevelIndex	= CreatedLODLevel->Level;

	MarkPackageDirty();

	return LevelIndex;
}

/** IsLODLevelValid
 *	Returns TRUE if the given LODLevel is one of the array entries.
 *	Intended for editor-time usage.
 *	Assumes that the given LODLevel will be in the [0..(NumLODLevels - 1)] range.
 *	
 *	@return FALSE if the requested LODLevel is not valid.
 *			TRUE if the requested LODLevel is valid.
 */
UBOOL UParticleEmitter::IsLODLevelValid(INT LODLevel)
{
	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* CheckLODLevel	= LODLevels(LODIndex);
		if (CheckLODLevel->Level == LODLevel)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * This will update the LOD of the particle in the editor.
 *
 * @see GetCurrentLODLevel(FParticleEmitterInstance* Instance)
 **/
void UParticleEmitter::EditorUpdateCurrentLOD(FParticleEmitterInstance* Instance)
{
	UParticleLODLevel*	CurrentLODLevel	= NULL;
	UParticleLODLevel*	Higher			= NULL;

	if (Instance->Component && Instance->Component->Template)
	{
		for (INT LevelIndex = 0; LevelIndex < LODLevels.Num(); LevelIndex++)
		{
			Higher	= LODLevels(LevelIndex);
			if (Higher->Level == Instance->Component->Template->EditorLODSetting)
			{
				CurrentLODLevel	= Higher;
				break;
			}
		}
	}

	if (CurrentLODLevel == NULL)
	{
		CurrentLODLevel = LODLevels(0);
	}
	Instance->CurrentLODLevelIndex	= CurrentLODLevel->Level;
	Instance->CurrentLODLevel		= CurrentLODLevel;
	Instance->EmitterDuration		= Instance->EmitterDurations(Instance->CurrentLODLevelIndex);
}


/** GetLODLevel
 *	Returns the given LODLevel. Intended for game-time usage.
 *	Assumes that the given LODLevel will be in the [0..# LOD levels] range.
 *	
 *	@param	LODLevel - the requested LOD level in the range [0..# LOD levels].
 *
 *	@return NULL if the requested LODLevel is not valid.
 *			The pointer to the requested UParticleLODLevel if valid.
 */
UParticleLODLevel* UParticleEmitter::GetLODLevel(INT LODLevel)
{
	if (LODLevel >= LODLevels.Num())
	{
		return NULL;
	}

	return LODLevels(LODLevel);
}

/**
 *	Autogenerate the lowest LOD level...
 *
 *	@param	bDuplicateHighest	If TRUE, make the level an exact copy of the highest
 *
 *	@return	UBOOL				TRUE if successful, FALSE if not.
 */
UBOOL UParticleEmitter::AutogenerateLowestLODLevel(UBOOL bDuplicateHighest)
{
	// Didn't find it?
	if (LODLevels.Num() == 1)
	{
		// We need to generate it...
		LODLevels.InsertZeroed(1, 1);
		UParticleLODLevel* LODLevel = ConstructObject<UParticleLODLevel>(UParticleLODLevel::StaticClass(), this);
		check(LODLevel);
		LODLevels(1)					= LODLevel;
		LODLevel->Level					= 1;
		LODLevel->ConvertedModules		= TRUE;
		LODLevel->PeakActiveParticles	= 0;

		// Grab LODLevel 0 for creation
		UParticleLODLevel* SourceLODLevel	= LODLevels(0);

		LODLevel->bEnabled				= SourceLODLevel->bEnabled;

		FLOAT Percentage	= 10.0f;
		if (SourceLODLevel->TypeDataModule)
		{
			UParticleModuleTypeDataTrail2*	Trail2TD	= Cast<UParticleModuleTypeDataTrail2>(SourceLODLevel->TypeDataModule);
			UParticleModuleTypeDataBeam2*	Beam2TD		= Cast<UParticleModuleTypeDataBeam2>(SourceLODLevel->TypeDataModule);

			if (Trail2TD || Beam2TD)
			{
				// For now, don't support LOD on beams and trails
				Percentage	= 100.0f;
			}
		}

		if (bDuplicateHighest == TRUE)
		{
			Percentage = 100.0f;
		}

		if (LODLevel->GenerateFromLODLevel(SourceLODLevel, Percentage) == FALSE)
		{
			warnf(TEXT("Failed to generate LOD level %d from LOD level 0"), 1);
			return FALSE;
		}

		MarkPackageDirty();
		return TRUE;
	}

	return TRUE;
}

/**
 *	CalculateMaxActiveParticleCount
 *	Determine the maximum active particles that could occur with this emitter.
 *	This is to avoid reallocation during the life of the emitter.
 *
 *	@return	TRUE	if the number was determined
 *			FALSE	if the number could not be determined
 */
UBOOL UParticleEmitter::CalculateMaxActiveParticleCount()
{
	INT	CurrMaxAPC = 0;

	UBOOL bIsBeamOrTrail = FALSE;
	INT MaxCount = 0;
	
	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels(LODIndex);
		if (LODLevel)
		{
			// Check for beams or trails
			if ((LODLevel->Level == 0) && (LODLevel->TypeDataModule != NULL))
			{
				UParticleModuleTypeDataBeam2* BeamTD = Cast<UParticleModuleTypeDataBeam2>(LODLevel->TypeDataModule);
				UParticleModuleTypeDataTrail2* TrailTD = Cast<UParticleModuleTypeDataTrail2>(LODLevel->TypeDataModule);
				if (BeamTD || TrailTD)
				{
					bIsBeamOrTrail = TRUE;
					if (BeamTD)
					{
						MaxCount = BeamTD->MaxBeamCount + 2;
					}
					if (TrailTD)
					{
						MaxCount = (TrailTD->MaxTrailCount * TrailTD->MaxParticleInTrailCount) + 2;
					}
				}
			}

			INT LODMaxAPC = LODLevel->CalculateMaxActiveParticleCount();
			if (bIsBeamOrTrail == TRUE)
			{
				LODLevel->PeakActiveParticles = MaxCount;
				LODMaxAPC = MaxCount;
			}

			if (LODMaxAPC > CurrMaxAPC)
			{
				if (LODIndex > 0)
				{
					// Check for a ridiculous difference in counts...
					if ((LODMaxAPC / CurrMaxAPC) > 2)
					{
						warnf(TEXT("MaxActiveParticleCount Discrepancy?\n\tLOD %2d, Emitter %16s"), LODIndex, *GetName());
					}
				}
				CurrMaxAPC = LODMaxAPC;
			}
		}
	}

	if ((GIsEditor == TRUE) && (CurrMaxAPC > 500))
	{
		//@todo. Added an option to the emitter to disable this warning - for 
		// the RARE cases where it is really required to render that many.
		warnf(TEXT("MaxCount = %4d for Emitter %s (%s)"),
			CurrMaxAPC, *(GetName()), GetOuter() ? *(GetOuter()->GetPathName()) : TEXT("????"));
	}

	return TRUE;
}

/**
 *	Retrieve the parameters associated with this particle system.
 *
 *	@param	ParticleSysParamList	The list of FParticleSysParams used in the system
 *	@param	ParticleParameterList	The list of ParticleParameter distributions used in the system
 */
void UParticleEmitter::GetParametersUtilized(TArray<FString>& ParticleSysParamList,
											 TArray<FString>& ParticleParameterList)
{
	// Clear the lists
	ParticleSysParamList.Empty();
	ParticleParameterList.Empty();

	TArray<UParticleModule*> ProcessedModules;
	ProcessedModules.Empty();

	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels(LODIndex);
		if (LODLevel)
		{
			INT FindIndex;
			// Grab that parameters from each module...
			check(LODLevel->RequiredModule);
			if (ProcessedModules.FindItem(LODLevel->RequiredModule, FindIndex) == FALSE)
			{
				LODLevel->RequiredModule->GetParticleSysParamsUtilized(ParticleSysParamList);
				LODLevel->RequiredModule->GetParticleParametersUtilized(ParticleParameterList);
				ProcessedModules.AddUniqueItem(LODLevel->RequiredModule);
			}

			check(LODLevel->SpawnModule);
			if (ProcessedModules.FindItem(LODLevel->SpawnModule, FindIndex) == FALSE)
			{
				LODLevel->SpawnModule->GetParticleSysParamsUtilized(ParticleSysParamList);
				LODLevel->SpawnModule->GetParticleParametersUtilized(ParticleParameterList);
				ProcessedModules.AddUniqueItem(LODLevel->SpawnModule);
			}

			if (LODLevel->TypeDataModule)
			{
				if (ProcessedModules.FindItem(LODLevel->TypeDataModule, FindIndex) == FALSE)
				{
					LODLevel->TypeDataModule->GetParticleSysParamsUtilized(ParticleSysParamList);
					LODLevel->TypeDataModule->GetParticleParametersUtilized(ParticleParameterList);
					ProcessedModules.AddUniqueItem(LODLevel->TypeDataModule);
				}
			}
			
			for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
			{
				UParticleModule* Module = LODLevel->Modules(ModuleIndex);
				if (Module)
				{
					if (ProcessedModules.FindItem(Module, FindIndex) == FALSE)
					{
						Module->GetParticleSysParamsUtilized(ParticleSysParamList);
						Module->GetParticleParametersUtilized(ParticleParameterList);
						ProcessedModules.AddUniqueItem(Module);
					}
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	UParticleSpriteEmitter implementation.
-----------------------------------------------------------------------------*/
void UParticleSpriteEmitter::PostLoad()
{
	Super::PostLoad();

	if (GIsUCCMake == TRUE)
	{
		return;
	}

	// Postload the materials
	for (INT LODIndex = 0; LODIndex < LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel = LODLevels(LODIndex);
		if (LODLevel)
		{
			UParticleModuleRequired* RequiredModule = LODLevel->RequiredModule;
			if (RequiredModule)
			{
				if (RequiredModule->Material)
				{
					RequiredModule->Material->ConditionalPostLoad();
				}
			}
		}
	}
}

void UParticleSpriteEmitter::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
}

FParticleEmitterInstance* UParticleSpriteEmitter::CreateInstance(UParticleSystemComponent* InComponent)
{
	if (LODLevels.Num() == 0)
	{
		return NULL;
	}

	FParticleEmitterInstance* Instance = 0;

	UParticleLODLevel* LODLevel	= GetLODLevel(0);
	check(LODLevel);

	if (LODLevel->TypeDataModule)
	{
		//@todo. This will NOT work for trails/beams!
		UParticleModuleTypeDataBase* TypeData = CastChecked<UParticleModuleTypeDataBase>(LODLevel->TypeDataModule);
		if (TypeData)
		{
			Instance = TypeData->CreateInstance(this, InComponent);
		}
	}
	else
	{
		if ((EParticleSubUVInterpMethod)(LODLevel->RequiredModule->InterpolationMethod) != PSUVIM_None)
		{
			check(InComponent);
			Instance = new FParticleSpriteSubUVEmitterInstance();
			check(Instance);
			Instance->InitParameters(this, InComponent);
		}
	}

	if (Instance == NULL)
	{
		check(InComponent);
		Instance = new FParticleSpriteEmitterInstance();
		check(Instance);
		Instance->InitParameters(this, InComponent);
	}

	check(Instance);

	Instance->CurrentLODLevelIndex	= 0;
	Instance->CurrentLODLevel		= LODLevels(Instance->CurrentLODLevelIndex);

	Instance->Init();
	return Instance;
}

// Sets up this sprite emitter with sensible defaults so we can see some particles as soon as its created.
void UParticleSpriteEmitter::SetToSensibleDefaults()
{
	PreEditChange(NULL);

	UParticleLODLevel* LODLevel = LODLevels(0);

	// Spawn rate
	LODLevel->SpawnModule->LODValidity = 1;
	UDistributionFloatConstant* SpawnRateDist = Cast<UDistributionFloatConstant>(LODLevel->SpawnModule->Rate.Distribution);
	if (SpawnRateDist)
	{
		SpawnRateDist->Constant = 20.f;
	}

	// Create basic set of modules

	// Lifetime module
	UParticleModuleLifetime* LifetimeModule = ConstructObject<UParticleModuleLifetime>(UParticleModuleLifetime::StaticClass(), GetOuter());
	UDistributionFloatUniform* LifetimeDist = Cast<UDistributionFloatUniform>(LifetimeModule->Lifetime.Distribution);
	if (LifetimeDist)
	{
		LifetimeDist->Min = 1.0f;
		LifetimeDist->Max = 1.0f;
		LifetimeDist->bIsDirty = TRUE;
	}
	LifetimeModule->LODValidity = 1;
	LODLevel->Modules.AddItem(LifetimeModule);

	// Size module
	UParticleModuleSize* SizeModule = ConstructObject<UParticleModuleSize>(UParticleModuleSize::StaticClass(), GetOuter());
	UDistributionVectorUniform* SizeDist = Cast<UDistributionVectorUniform>(SizeModule->StartSize.Distribution);
	if (SizeDist)
	{
		SizeDist->Min = FVector(25.f, 25.f, 25.f);
		SizeDist->Max = FVector(25.f, 25.f, 25.f);
		SizeDist->bIsDirty = TRUE;
	}
	SizeModule->LODValidity = 1;
	LODLevel->Modules.AddItem(SizeModule);

	// Initial velocity module
	UParticleModuleVelocity* VelModule = ConstructObject<UParticleModuleVelocity>(UParticleModuleVelocity::StaticClass(), GetOuter());
	UDistributionVectorUniform* VelDist = Cast<UDistributionVectorUniform>(VelModule->StartVelocity.Distribution);
	if (VelDist)
	{
		VelDist->Min = FVector(-10.f, -10.f, 50.f);
		VelDist->Max = FVector(10.f, 10.f, 100.f);
		VelDist->bIsDirty = TRUE;
	}
	VelModule->LODValidity = 1;
	LODLevel->Modules.AddItem(VelModule);

	PostEditChange(NULL);
}

/*-----------------------------------------------------------------------------
	UParticleSystem implementation.
-----------------------------------------------------------------------------*/
/** 
 *	Return the currently set LOD method
 */
BYTE UParticleSystem::GetCurrentLODMethod()
{
	return LODMethod;
}

/**
 *	Return the number of LOD levels for this particle system
 *
 *	@return	The number of LOD levels in the particle system
 */
INT UParticleSystem::GetLODLevelCount()
{
	return LODDistances.Num();
}

/**
 *	Return the distance for the given LOD level
 *
 *	@param	LODLevelIndex	The LOD level that the distance is being retrieved for
 *
 *	@return	-1.0f			If the index is invalid
 *			Distance		The distance set for the LOD level
 */
FLOAT UParticleSystem::GetLODDistance(INT LODLevelIndex)
{
	if (LODLevelIndex >= LODDistances.Num())
	{
		return -1.0f;
	}

	return LODDistances(LODLevelIndex);
}

/**
 *	Set the LOD method
 *
 *	@param	InMethod		The desired method
 */
void UParticleSystem::SetCurrentLODMethod(BYTE InMethod)
{
	LODMethod = (ParticleSystemLODMethod)InMethod;
}

/**
 *	Set the distance for the given LOD index
 *
 *	@param	LODLevelIndex	The LOD level to set the distance ofr
 *	@param	InDistance		The distance to set
 *
 *	@return	TRUE			If successful
 *			FALSE			Invalid LODLevelIndex
 */
UBOOL UParticleSystem::SetLODDistance(INT LODLevelIndex, FLOAT InDistance)
{
	if (LODLevelIndex >= LODDistances.Num())
	{
		return FALSE;
	}

	LODDistances(LODLevelIndex) = InDistance;

	return TRUE;
}

/**
 *	Handle edited properties
 *
 *	@param	PropertyThatChanged		The property that was changed
 */
void UParticleSystem::PostEditChange(UProperty* PropertyThatChanged)
{
	UpdateTime_Delta = 1.0f / UpdateTime_FPS;

	for (TObjectIterator<UParticleSystemComponent> It;It;++It)
	{
		if (It->Template == this)
		{
			It->UpdateInstances();
		}
	}

	ThumbnailImageOutOfDate = TRUE;

	Super::PostEditChange(PropertyThatChanged);
}

void UParticleSystem::PreSave()
{
	Super::PreSave();
}

void UParticleSystem::PostLoad()
{
	Super::PostLoad();

	//@todo. Put this in a better place??
	bool bHadDeprecatedEmitters = FALSE;

	// Remove any old emitters
	UBOOL IsLit = FALSE;
	for (INT i = Emitters.Num() - 1; i >= 0; i--)
	{
		UParticleEmitter* Emitter = Emitters(i);
		if (Emitter == NULL)
		{
			// Empty emitter slots are ok with cooked content.
			if( !GUseSeekFreeLoading )
			{
				warnf(TEXT("ParticleSystem contains empty emitter slots - %s"), *GetFullName());
			}
			continue;
		}

		Emitter->ConditionalPostLoad();

		if (Emitter->IsA(UParticleSpriteEmitter::StaticClass()))
		{
			UBOOL bIsMeshEmitter = FALSE;
			UMaterial* UMat = NULL;
			UParticleSpriteEmitter* SpriteEmitter = Cast<UParticleSpriteEmitter>(Emitter);

			UParticleLODLevel* LODLevel = SpriteEmitter->LODLevels(0);
			check(LODLevel);
			
			LODLevel->ConditionalPostLoad();
			if (LODLevel->TypeDataModule)
			{
				//@todo. Check for lighting on mesh elements!
				UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
				if (MeshTD)
				{
					bIsMeshEmitter = TRUE;

					UBOOL bQuickOut = FALSE;
					UStaticMesh* Mesh = MeshTD->Mesh;
					if (Mesh)
					{
						Mesh->ConditionalPostLoad();
						for (INT LODIndex = 0; (LODIndex < Mesh->LODModels.Num()) && (bQuickOut == FALSE); LODIndex++)
						{
							FStaticMeshRenderData* LOD = &(Mesh->LODModels(LODIndex));
							for (INT MatIndex = 0; (MatIndex < LOD->Elements.Num()) && (bQuickOut == FALSE); MatIndex++)
							{
								UMaterialInterface* MatInst = LOD->Elements(MatIndex).Material;
								if (MatInst)
								{
									MatInst->ConditionalPostLoad();
									UMat = MatInst->GetMaterial();
								}
							}
						}
					}
				}
			}

			if (UMat && (UMat->LightingModel != MLM_Unlit))
			{
				IsLit = TRUE;

#if 0
					FString TempName = GetPathName();
					TempName.Replace(TEXT("."), TEXT(", "));
					debugf(NAME_ParticleWarn, TEXT("LIT: %s, %s, %s, %d"),*TempName, *UMat->GetName(), bIsMeshEmitter ? TEXT("Y") : TEXT("N"), i);
#endif
			}
#if defined(_PE_CHECK_LODLEVELS_)
			UBOOL bQuickOut = FALSE;
			for (INT LODLevelIndex = 0; (LODLevelIndex < SpriteEmitter->LODLevels.Num()) && (bQuickOut == FALSE); LODLevelIndex++)
			{
				UParticleLODLevel* LODLevel = SpriteEmitter->LODLevels(LODLevelIndex);
				if (LODLevel && LODLevel->TypeDataModule)
				{
					//@todo. Check for lighting on mesh elements!
					UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);
					if (MeshTD)
					{
						UStaticMesh* Mesh = MeshTD->Mesh;
						if (Mesh)
						{
							for (INT LODIndex = 0; (LODIndex < Mesh->LODModels.Num()) && (bQuickOut == FALSE); LODIndex++)
							{
								FStaticMeshRenderData* LOD = &(Mesh->LODModels(LODIndex));
								for (INT MatIndex = 0; (MatIndex < LOD->Elements.Num()) && (bQuickOut == FALSE); MatIndex++)
								{
									UMaterialInterface* MatInst = LOD->Elements(MatIndex).Material;
									if (MatInst)
									{
										MatInst->ConditionalPostLoad();
										UMaterial* Mat = MatInst->GetMaterial();
										if (Mat && Mat->LightingModel != MLM_Unlit)
										{
											IsLit = TRUE;
											if (GIsEditor == TRUE)
											{
												FString TempName = GetPathName();
												TempName.Replace(TEXT("."), TEXT(", "));
												debugf(NAME_ParticleWarn, TEXT("LIT: %s, %s, Y, %d"),*TempName, *Mat->GetName(), i);
											}
											bQuickOut = TRUE;
										}
									}
								}
							}
						}
					}
				}
			}
#endif	//#if defined(_PE_CHECK_LODLEVELS_)

#if !FINAL_RELEASE
#if CONSOLE
			// Do not cook out disabled emitter...
			UBOOL bDisabledEmitter = TRUE;
			for (INT LODLevelIndex = 0; (LODLevelIndex < SpriteEmitter->LODLevels.Num()) && (bDisabledEmitter == TRUE); LODLevelIndex++)
			{
				UParticleLODLevel* DisabledLODLevel = SpriteEmitter->LODLevels(LODLevelIndex);
				if (DisabledLODLevel)
				{
					if (DisabledLODLevel->bEnabled == TRUE)
					{
						bDisabledEmitter = FALSE;
					}
				}
			}

			if (bDisabledEmitter)
			{
				// We don't actually delete it, we just clear it's spot in the emitter array
				//warnf(NAME_ParticleWarn,  TEXT("Emitter %2d disabled in PSys   %s"), i, *(GetPathName()));
			}
#endif	//#if CONSOLE
#endif	//#if !FINAL_RELEASE

			//@todo. Move this into the editor and serialize?
			bHasPhysics = FALSE;
			for (INT LODIndex = 0; (LODIndex < Emitter->LODLevels.Num()) && (bHasPhysics == FALSE); LODIndex++)
			{
				//@todo. This is a temporary fix for emitters that apply physics.
				// Check for collision modules with bApplyPhysics set to TRUE
				UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
				if (LODLevel)
				{
					for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
					{
						UParticleModuleCollision* CollisionModule = Cast<UParticleModuleCollision>(LODLevel->Modules(ModuleIndex));
						if (CollisionModule)
						{
							if (CollisionModule->bApplyPhysics == TRUE)
							{
								bHasPhysics = TRUE;
								break;
							}
						}
					}
				}
			}
		}
	}
	bLit = IsLit;

	if ((bHadDeprecatedEmitters || (GetLinker() && (GetLinker()->Ver() < 204))) && CurveEdSetup)
	{
		CurveEdSetup->ResetTabs();
	}

	// Add default LOD Distances
	if( LODDistances.Num() == 0 && Emitters.Num() > 0 )
	{
		UParticleEmitter* Emitter = Emitters(0);
		if (Emitter)
		{
			LODDistances.Add(Emitter->LODLevels.Num());
			for (INT LODIndex = 0; LODIndex < LODDistances.Num(); LODIndex++)
			{
				LODDistances(LODIndex) = LODIndex * 2500.0f;
			}
		}
	}

//	if (GetLinker() && (GetLinker()->Ver() < VER_PARTICLE_LOD_DISTANCE_FIXUP))
	// Due to there still being some ways that LODLevel counts get mismatched,
	// when loading in the editor LOD levels will always be checked and fixed
	// up... This can be removed once all the edge cases that lead to the
	// problem are found and fixed.
	if (GIsEditor && !GIsCooking && !GIsUCCMake)
	{
		// Fix the LOD distance array and mismatched lod levels
		INT LODCount_0 = -1;
		for (INT EmitterIndex = 0; EmitterIndex < Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter* Emitter  = Emitters(EmitterIndex);
			if (Emitter)
			{
				if (LODCount_0 == -1)
				{
					LODCount_0 = Emitter->LODLevels.Num();
				}
				else
				{
					INT EmitterLODCount = Emitter->LODLevels.Num();
					if (EmitterLODCount != LODCount_0)
					{
						warnf(TEXT("Emitter %d has mismatched LOD level count - expected %d, found %d. PS = %s"),
							EmitterIndex, LODCount_0, EmitterLODCount, *GetPathName());
						warnf(TEXT("Fixing up now... Pacakge = %s"), *(GetOutermost()->GetPathName()));

						if (EmitterLODCount > LODCount_0)
						{
							Emitter->LODLevels.Remove(LODCount_0, EmitterLODCount - LODCount_0);
						}
						else
						{
							for (INT NewLODIndex = EmitterLODCount; NewLODIndex < LODCount_0; NewLODIndex++)
							{
								if (Emitter->CreateLODLevel(NewLODIndex) != NewLODIndex)
								{
									warnf(TEXT("Failed to add LOD level %s"), NewLODIndex);
								}
							}
						}
					}
				}
			}
		}

		if (LODCount_0 > 0)
		{
			if (LODDistances.Num() < LODCount_0)
			{
				for (INT DistIndex = LODDistances.Num(); DistIndex < LODCount_0; DistIndex++)
				{
					FLOAT Distance = DistIndex * 2500.0f;
					LODDistances.AddItem(Distance);
				}
			}
			else
			if (LODDistances.Num() > LODCount_0)
			{
				LODDistances.Remove(LODCount_0, LODDistances.Num() - LODCount_0);
			}
		}
		else
		{
			LODDistances.Empty();
		}
	}

	// Need to fix-up any issues w/ mismatched interpolation methods on the LODs in emitters
	if (GetLinker() && (GetLinker()->Ver() < VER_EMITTER_INTERPOLATIONMETHOD_FIXUP))
	{
		if (GIsEditor && !GIsCooking && !GIsUCCMake)
		{
			for (INT EmitterIndex = 0; EmitterIndex < Emitters.Num(); EmitterIndex++)
			{
				UParticleEmitter* Emitter  = Emitters(EmitterIndex);
				if (Emitter)
				{
					UParticleLODLevel* LODLevel0 = Emitter->LODLevels(0);
					if (LODLevel0 && LODLevel0->RequiredModule)
					{
						for (INT LODIndex = 1; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
						{
							UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
							if (LODLevel && LODLevel->RequiredModule)
							{
								if (LODLevel0->RequiredModule->InterpolationMethod == PSUVIM_None)
								{
									if (LODLevel->RequiredModule->InterpolationMethod != PSUVIM_None)
									{
										warnf(TEXT("Fixing up mismatched InterpolationMethod in ParticleSystem %s"), *GetPathName());
										LODLevel->RequiredModule->InterpolationMethod = PSUVIM_None;
									}
								}
								else
								{
									if (LODLevel->RequiredModule->InterpolationMethod == PSUVIM_None)
									{
										warnf(TEXT("Fixing up mismatched InterpolationMethod in ParticleSystem %s"), *GetPathName());
										LODLevel->RequiredModule->InterpolationMethod = LODLevel0->RequiredModule->InterpolationMethod;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Since LOD checks have been performed EVERY frame for a while now, 
	// all LODDistance check times are being reset to 0.0f to contiue doing so.
	if (GetLinker() && (GetLinker()->Ver() < VER_PARTICLE_LOD_CHECK_DISTANCE_TIME_FIX))
	{
		LODDistanceCheckTime = 0.0f;
	}
}

void UParticleSystem::UpdateColorModuleClampAlpha(UParticleModuleColorBase* ColorModule)
{
	if (ColorModule)
	{
		ColorModule->RemoveModuleCurvesFromEditor(CurveEdSetup);
		ColorModule->AddModuleCurvesToEditor(CurveEdSetup);
	}
}

/**
 *	CalculateMaxActiveParticleCounts
 *	Determine the maximum active particles that could occur with each emitter.
 *	This is to avoid reallocation during the life of the emitter.
 *
 *	@return	TRUE	if the numbers were determined for each emitter
 *			FALSE	if not be determined
 */
UBOOL UParticleSystem::CalculateMaxActiveParticleCounts()
{
	UBOOL bSuccess = TRUE;

	for (INT EmitterIndex = 0; EmitterIndex < Emitters.Num(); EmitterIndex++)
	{
		UParticleEmitter* Emitter = Emitters(EmitterIndex);
		if (Emitter)
		{
			if (Emitter->CalculateMaxActiveParticleCount() == FALSE)
			{
				bSuccess = FALSE;
			}
		}
	}

	return bSuccess;
}

/**
 *	Retrieve the parameters associated with this particle system.
 *
 *	@param	ParticleSysParamList	The list of FParticleSysParams used in the system
 *	@param	ParticleParameterList	The list of ParticleParameter distributions used in the system
 */
void UParticleSystem::GetParametersUtilized(TArray<TArray<FString> >& ParticleSysParamList,
											TArray<TArray<FString> >& ParticleParameterList)
{
	ParticleSysParamList.Empty();
	ParticleParameterList.Empty();

	for (INT EmitterIndex = 0; EmitterIndex < Emitters.Num(); EmitterIndex++)
	{
		INT CheckIndex;
		CheckIndex = ParticleSysParamList.AddZeroed();
		check(CheckIndex == EmitterIndex);
		CheckIndex = ParticleParameterList.AddZeroed();
		check(CheckIndex == EmitterIndex);

		UParticleEmitter* Emitter = Emitters(EmitterIndex);
		if (Emitter)
		{
			Emitter->GetParametersUtilized(
				ParticleSysParamList(EmitterIndex),
				ParticleParameterList(EmitterIndex));
		}
	}
}

/*-----------------------------------------------------------------------------
	UParticleSystemComponent implementation.
-----------------------------------------------------------------------------*/
void UParticleSystemComponent::CheckForErrors()
{
	for (INT IPIndex = 0; IPIndex < InstanceParameters.Num(); IPIndex++)
	{
		FParticleSysParam& Param = InstanceParameters(IPIndex);
		if (Param.ParamType == PSPT_Actor)
		{
			if (Param.Actor != NULL)
			{
				if ((Param.Actor->bNoDelete == FALSE) && (Param.Actor->bStatic == FALSE))

				{
					GWarn->MapCheck_Add(MCTYPE_WARNING, this, 
						*FString::Printf(LocalizeSecure(LocalizeUnrealEd("PSysComp_Error_BadActorRef"), 
						IPIndex, *GetPathName())), MCACTION_NONE);
				}
			}
			else
			{
				GWarn->MapCheck_Add(MCTYPE_WARNING, this, 
					*FString::Printf(LocalizeSecure(LocalizeUnrealEd("PSysComp_Error_EmptyActorRef"), 
					IPIndex, *GetPathName())), MCACTION_NONE);
			}
		}
		else
		if (Param.ParamType == PSPT_Material)
		{
			if (Param.Material == NULL)
			{
				GWarn->MapCheck_Add(MCTYPE_WARNING, this, 
					*FString::Printf(LocalizeSecure(LocalizeUnrealEd("PSysComp_Error_EmptyMaterialRef"), 
					IPIndex, *GetPathName())), MCACTION_NONE);
			}
		}
	}
}

void UParticleSystemComponent::PostLoad()
{
	Super::PostLoad();

	if (Template)
	{
		Template->ConditionalPostLoad();
	}
	bIsViewRelevanceDirty = TRUE;

	// Initialize the system to avoid hitching
	//@todo.SAS. Do we want to make this a flag on the PSys?
	InitializeSystem();
}

void UParticleSystemComponent::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	
	// Take instance particle count/ size into account.
	for (INT InstanceIndex = 0; InstanceIndex < EmitterInstances.Num(); InstanceIndex++)
	{
		FParticleEmitterInstance* EmitterInstance = EmitterInstances(InstanceIndex);
		if( EmitterInstance != NULL )
		{
			INT Num, Max;
			EmitterInstance->GetAllocatedSize(Num, Max);
			Ar.CountBytes(Num, Max);
		}
	}
}

void UParticleSystemComponent::BeginDestroy()
{
	Super::BeginDestroy();
#if PARTICLES_USE_DOUBLE_BUFFERING

	// Release the resources...
	for (INT BuffIndex = 0; BuffIndex < 2; BuffIndex++)
	{
		if (DynamicDataBuffers[BuffIndex])
		{
			DynamicDataBuffers[BuffIndex]->ReleaseResources();
		}
	}
	// Set a fence...
	ReleaseResourcesFence = new FRenderCommandFence();
	check(ReleaseResourcesFence);
	ReleaseResourcesFence->BeginFence();
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	ResetParticles(TRUE);
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

UBOOL UParticleSystemComponent::IsReadyForFinishDestroy()
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	UBOOL bSuperIsReady = Super::IsReadyForFinishDestroy();
	UBOOL bIsReady = TRUE;
	if (ReleaseResourcesFence)
	{
		bIsReady = (ReleaseResourcesFence->GetNumPendingFences() == 0);
	}

	if (DeferredReleaseItems)
	{
		// Clear out any pending deletions from previous frames...
		// Need to do this prior to quit-outting of the function to properly cleanup.
		for (INT DeferredIndex = DeferredReleaseItems->DeferredReleases.Num() - 1; DeferredIndex >= 0; DeferredIndex--)
		{
			FParticleDeferredReleaseResources& DeferredRelease = DeferredReleaseItems->DeferredReleases(DeferredIndex);
			if (DeferredRelease.ReleaseFence && (DeferredRelease.ReleaseFence->GetNumPendingFences() == 0))
			{
				delete DeferredRelease.DynamicData;
				DeferredRelease.DynamicData = NULL;
				delete DeferredRelease.EmitterDynamicData;
				DeferredRelease.EmitterDynamicData = NULL;
				delete DeferredRelease.ReleaseFence;
				DeferredRelease.ReleaseFence = NULL;
				DeferredReleaseItems->DeferredReleases.Remove(DeferredIndex);
			}
		}
		if (DeferredReleaseItems->DeferredReleases.Num() > 0)
		{
			bIsReady = FALSE;
		}
	}
	return (bSuperIsReady && bIsReady);
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	return TRUE;
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

void UParticleSystemComponent::FinishDestroy()
{
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	check(DynamicDataBuffers[0] == NULL);
	check(DynamicDataBuffers[1] == NULL);
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

	GParticleDataManager.RemoveParticleSystemComponent(this);
	for (INT EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
	{
		FParticleEmitterInstance* EmitInst = EmitterInstances(EmitterIndex);
		if (EmitInst)
		{
			delete EmitInst;
			EmitterInstances(EmitterIndex) = NULL;
		}
	}
#if PARTICLES_USE_DOUBLE_BUFFERING
	ResetParticles(TRUE);
	for (INT DDBuffIndex = 0; DDBuffIndex < 2; DDBuffIndex++)
	{
		// Clean up the dynamic data buffers...
		delete DynamicDataBuffers[DDBuffIndex];
		DynamicDataBuffers[DDBuffIndex] = NULL;
	}

	if (DeferredReleaseItems)
	{
		for (INT DeferredIndex = DeferredReleaseItems->DeferredReleases.Num() - 1; DeferredIndex >= 0; DeferredIndex--)
		{
			FParticleDeferredReleaseResources& DeferredRelease = DeferredReleaseItems->DeferredReleases(DeferredIndex);
			if (DeferredRelease.ReleaseFence)
			{
				check((DeferredRelease.ReleaseFence->GetNumPendingFences() == 0));
			}
			delete DeferredRelease.DynamicData;
			DeferredRelease.DynamicData = NULL;
			delete DeferredRelease.EmitterDynamicData;
			DeferredRelease.EmitterDynamicData = NULL;
			delete DeferredRelease.ReleaseFence;
			DeferredRelease.ReleaseFence = NULL;
			DeferredReleaseItems->DeferredReleases.Remove(DeferredIndex);
		}
	}
	delete DeferredReleaseItems;
	DeferredReleaseItems = NULL;

	if (ReleaseResourcesFence)
	{
		delete ReleaseResourcesFence;
		ReleaseResourcesFence = NULL;
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
	Super::FinishDestroy();
}

// Collision Handling...
UBOOL UParticleSystemComponent::SingleLineCheck(FCheckResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, DWORD TraceFlags, const FVector& Extent)
{
	check(GWorld);

	return GWorld->SingleLineCheck(Hit, SourceActor, End, Start, TraceFlags, Extent);
}

void UParticleSystemComponent::Attach()
{
	// NULL out template if we're not allowing particles. This is not done in the Editor to avoid clobbering content via PIE.
	if( !GIsAllowingParticles && !GIsEditor )
	{
		Template = NULL;
	}

	if (Template)
	{
		this->bAcceptsLights = Template->bLit;
		if (Template->bHasPhysics)
		{
			TickGroup = TG_PreAsyncWork;
			
			AEmitter* EmitterOwner = Cast<AEmitter>(GetOwner());
			if (EmitterOwner)
			{
				EmitterOwner->TickGroup = TG_PreAsyncWork;
			}
		}

		if (LODLevel == -1)
		{
			// Force it to LODLevel 0
			LODLevel = 0;
		}
	}

	Super::Attach();

	if (Template && bAutoActivate && (EmitterInstances.Num() == 0 || bResetOnDetach))
	{
		InitializeSystem();
	}

	if (Template && (bIsActive == FALSE) && (bAutoActivate == TRUE)&& (EmitterInstances.Num() > 0) && (bWasDeactivated == FALSE))
	{
		SetActive(TRUE);
	}

	if (Template)
	{
//		if (SceneInfo != NULL)
		{
			GParticleDataManager.AddParticleSystemComponent(this);
		}
	}

	bJustAttached = TRUE;
}

void UParticleSystemComponent::UpdateTransform()
{
	if (bIsActive)
	{
		Super::UpdateTransform();
		GParticleDataManager.AddParticleSystemComponent(this);
	}
}

void UParticleSystemComponent::Detach( UBOOL bWillReattach )
{
	if (bResetOnDetach)
	{
		// Empty the EmitterInstance array.
		ResetParticles();
	}
	else
	{
		// tell emitter instances that we were detached, but don't clear them
		for (INT InstanceIndex = 0; InstanceIndex < EmitterInstances.Num(); InstanceIndex++)
		{
			FParticleEmitterInstance* EmitterInstance = EmitterInstances(InstanceIndex);
			if (EmitterInstance != NULL)
			{
				EmitterInstance->RemovedFromScene();
			}
		}
	}

	if (GIsGame == TRUE)
	{
		GParticleDataManager.RemoveParticleSystemComponent(this);
	}

	Super::Detach( bWillReattach );
}

void UParticleSystemComponent::UpdateDynamicData()
{
	if (SceneInfo)
	{
		FParticleSystemSceneProxy* SceneProxy = (FParticleSystemSceneProxy*)Scene_GetProxyFromInfo(SceneInfo);
		UpdateDynamicData(SceneProxy);
	}
}


/**
 * Static: Supplied with a chunk of replay data, this method will create dynamic emitter data that can
 * be used to render the particle system
 *
 * @param	EmitterInstance		Emitter instance this replay is playing on
 * @param	EmitterReplayData	Incoming replay data of any time, cannot be NULL
 * @param	bSelected			TRUE if the particle system is currently selected
 *
 * @return	The newly created dynamic data, or NULL on failure
 */
FDynamicEmitterDataBase* UParticleSystemComponent::CreateDynamicDataFromReplay( FParticleEmitterInstance* EmitterInstance, 
	const FDynamicEmitterReplayDataBase* EmitterReplayData, UBOOL bSelected )
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	checkf(0, TEXT("CreateDynamicDataFromReplay> Not supported yet for double-buffering!"));
	return NULL;
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	check( EmitterReplayData != NULL );

	// Allocate the appropriate type of emitter data
	FDynamicEmitterDataBase* EmitterData = NULL;

	switch( EmitterReplayData->eEmitterType )
	{
		case DET_Sprite:
			{
				// Allocate the dynamic data
				FDynamicSpriteEmitterData* NewEmitterData = ::new FDynamicSpriteEmitterData();

				// Fill in the source data
				const FDynamicSpriteEmitterReplayData* SpriteEmitterReplayData =
					static_cast< const FDynamicSpriteEmitterReplayData* >( EmitterReplayData );
				NewEmitterData->Source = *SpriteEmitterReplayData;

				// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
				NewEmitterData->Init( bSelected );

				EmitterData = NewEmitterData;
			}
			break;

		case DET_SubUV:
			{
				// Allocate the dynamic data
				FDynamicSubUVEmitterData* NewEmitterData = ::new FDynamicSubUVEmitterData();

				// Fill in the source data
				const FDynamicSubUVEmitterReplayData* SubUVEmitterReplayData =
					static_cast< const FDynamicSubUVEmitterReplayData* >( EmitterReplayData );
				NewEmitterData->Source = *SubUVEmitterReplayData;

				// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
				NewEmitterData->Init( bSelected );

				EmitterData = NewEmitterData;
			}
			break;

		case DET_Mesh:
			{
				// Allocate the dynamic data
				FDynamicMeshEmitterData* NewEmitterData = ::new FDynamicMeshEmitterData();

				// Fill in the source data
				const FDynamicMeshEmitterReplayData* MeshEmitterReplayData =
					static_cast< const FDynamicMeshEmitterReplayData* >( EmitterReplayData );
				NewEmitterData->Source = *MeshEmitterReplayData;

				// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.

				// @todo: Currently we're assuming the original emitter instance is bound to the same mesh as
				//        when the replay was generated (safe), and various mesh/material indices are intact.  If
				//        we ever support swapping meshes/material on the fly, we'll need cache the mesh
				//        reference and mesh component/material indices in the actual replay data.

				if( EmitterInstance != NULL )
				{
					FParticleMeshEmitterInstance* MeshEmitterInstance =
						static_cast< FParticleMeshEmitterInstance* >( EmitterInstance );

					if( MeshEmitterInstance->MeshComponentIndex != -1 &&
						MeshEmitterInstance->MeshComponentIndex < MeshEmitterInstance->Component->SMComponents.Num() )
					{
						UStaticMeshComponent* MeshComponent =
							MeshEmitterInstance->Component->SMComponents( MeshEmitterInstance->MeshComponentIndex );
						if( MeshComponent != NULL )
						{
							NewEmitterData->Init(
								bSelected,
								MeshEmitterInstance,
								MeshEmitterInstance->MeshTypeData->Mesh,
								MeshComponent,
								FALSE );	// Use NxFluid?
						}
					}

					EmitterData = NewEmitterData;
				}
			}
			break;

		case DET_Beam2:
			{
				// Allocate the dynamic data
				FDynamicBeam2EmitterData* NewEmitterData = ::new FDynamicBeam2EmitterData();

				// Fill in the source data
				const FDynamicBeam2EmitterReplayData* Beam2EmitterReplayData =
					static_cast< const FDynamicBeam2EmitterReplayData* >( EmitterReplayData );
				NewEmitterData->Source = *Beam2EmitterReplayData;

				// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
				NewEmitterData->Init( bSelected );

				EmitterData = NewEmitterData;
			}
			break;

		case DET_Trail2:
			{
				// Allocate the dynamic data
				FDynamicTrail2EmitterData* NewEmitterData = ::new FDynamicTrail2EmitterData();

				// Fill in the source data
				const FDynamicTrail2EmitterReplayData* Trail2EmitterReplayData =
					static_cast< const FDynamicTrail2EmitterReplayData* >( EmitterReplayData );
				NewEmitterData->Source = *Trail2EmitterReplayData;

				// Setup dynamic render data.  Only call this AFTER filling in source data for the emitter.
				NewEmitterData->Init( bSelected );

				EmitterData = NewEmitterData;
			}
			break;

		default:
			{
				// @todo: Support capture of other particle system types
			}
			break;
	}

	return EmitterData;
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}



/**
 * Creates dynamic particle data for rendering the particle system this frame.  This function
 * handle creation of dynamic data for regularly simulated particles, but also handles capture
 * and playback of particle replay data.
 *
 * @return	Returns the dynamic data to render this frame
 */
FParticleDynamicData* UParticleSystemComponent::CreateDynamicData()
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	// Allocate the buffer if needed...
	if (DynamicDataBuffers[GameThreadDynamicDataIndex] == NULL)
	{
		DynamicDataBuffers[GameThreadDynamicDataIndex] = new FParticleDynamicBufferedData();
		check(DynamicDataBuffers[GameThreadDynamicDataIndex]);
	}

#if 1
	// Update the dynamic data contents... this will handle changing templates.
	DynamicDataBuffers[GameThreadDynamicDataIndex]->UpdateTemplate(this);
#else
	// This will reallocate each frame, mimicking the old behavior...
	DynamicDataBuffers[GameThreadDynamicDataIndex]->bForceTemplateReset = TRUE;
	DynamicDataBuffers[GameThreadDynamicDataIndex]->UpdateTemplate(this);
#endif
	// Grab the pointer to the actual DynamicData
	FParticleDynamicData* ParticleDynamicData = DynamicDataBuffers[GameThreadDynamicDataIndex]->DynamicData;
	
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	FParticleDynamicData* ParticleDynamicData = new FParticleDynamicData();
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

#if PARTICLES_USE_DOUBLE_BUFFERING
		//@todo.SAS. Support double buffering for Replay!!!
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
	if( ReplayState == PRS_Replaying )
	{
		// Do we have any replay data to play back?
		UParticleSystemReplay* ReplayData = FindReplayClipForIDNumber( ReplayClipIDNumber );
		if( ReplayData != NULL )
		{
			// Make sure the current frame index is in a valid range
			if( ReplayData->Frames.IsValidIndex( ReplayFrameIndex ) )
			{
				// Grab the current particle system replay frame
				const FParticleSystemReplayFrame& CurReplayFrame = ReplayData->Frames( ReplayFrameIndex );


				// Fill the emitter dynamic buffers with data from our replay
				ParticleDynamicData->DynamicEmitterDataArray.Empty( CurReplayFrame.Emitters.Num() );
				for( INT CurEmitterIndex = 0; CurEmitterIndex < CurReplayFrame.Emitters.Num(); ++CurEmitterIndex )
				{
					const FParticleEmitterReplayFrame& CurEmitter = CurReplayFrame.Emitters( CurEmitterIndex );

					const FDynamicEmitterReplayDataBase* CurEmitterReplay = CurEmitter.FrameState;
					check( CurEmitterReplay != NULL );

					// Grab the original emitter instance for that this replay was generated from
					// @todo: This is currently only needed for mesh emitters
					FParticleEmitterInstance* EmitterInst = NULL;
					if( EmitterInstances.IsValidIndex( CurEmitter.OriginalEmitterIndex ) )
					{
						EmitterInst = EmitterInstances( CurEmitter.OriginalEmitterIndex );
					}

					// Fill dynamic data from the replay frame data for this emitter so we can render it
					FDynamicEmitterDataBase* NewDynamicEmitterData =
						CreateDynamicDataFromReplay( EmitterInst, CurEmitterReplay, IsOwnerSelected() );
					if( NewDynamicEmitterData != NULL )
					{
						ParticleDynamicData->DynamicEmitterDataArray.AddItem( NewDynamicEmitterData );
					}
				}
			}
		}
	}
	else
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
	{
		FParticleSystemReplayFrame* NewReplayFrame = NULL;
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
		if( ReplayState == PRS_Capturing )
		{
			// If we don't have any replay data for this component yet, create some now
			UParticleSystemReplay* ReplayData = FindReplayClipForIDNumber( ReplayClipIDNumber );
			if( ReplayData == NULL )
			{
				// Create a new replay clip!
				ReplayData = ConstructObject< UParticleSystemReplay >( UParticleSystemReplay::StaticClass(), this );

				// Set the clip ID number
				ReplayData->ClipIDNumber = ReplayClipIDNumber;

				// Add this to the component's list of clips
				ReplayClips.AddItem( ReplayData );

				// We're modifying the component by adding a new replay clip
				MarkPackageDirty();
			}


			// Add a new frame!
			{
				const INT NewFrameIndex = ReplayData->Frames.Num();
				new( ReplayData->Frames ) FParticleSystemReplayFrame( EC_EventParm );
				NewReplayFrame = &ReplayData->Frames( NewFrameIndex );

				// We're modifying the component by adding a new frame
				MarkPackageDirty();
			}
		}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

		// Is the particle system allowed to run?
		if( bForcedInActive == FALSE )
		{
#if PARTICLES_USE_DOUBLE_BUFFERING
			if (ParticleDynamicData != NULL)
			{
				check(ParticleDynamicData->DynamicEmitterDataArray.Num() == EmitterInstances.Num());

				for (INT EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
				{
					FDynamicEmitterDataBase* DynamicEmitterData = ParticleDynamicData->DynamicEmitterDataArray(EmitterIndex);
					FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
					if (EmitterInst)
					{
#if !FINAL_RELEASE
						if (DynamicEmitterData)
						{
							switch (DynamicEmitterData->GetSource().eEmitterType)
							{
							case DET_Sprite:
								{
									FParticleSpriteEmitterInstance* TestInst = CastEmitterInstance<FParticleSpriteEmitterInstance>(EmitterInst);
									check(TestInst);
								}
								break;
							case DET_SubUV:
								{
									FParticleSpriteSubUVEmitterInstance* TestInst = CastEmitterInstance<FParticleSpriteSubUVEmitterInstance>(EmitterInst);
									check(TestInst);
								}
								break;
							case DET_Mesh:
								{
									FParticleMeshEmitterInstance* TestInst = CastEmitterInstance<FParticleMeshEmitterInstance>(EmitterInst);
									check(TestInst);
									static UBOOL s_bSkipMeshes = FALSE;
									if (s_bSkipMeshes == TRUE)
									{
										this->HandleDynamicDataDeletion(NULL, DynamicEmitterData);
										DynamicEmitterData = NULL;
										ParticleDynamicData->DynamicEmitterDataArray(EmitterIndex) = NULL;
									}
								}
								break;
							case DET_Beam2:
								{
									FParticleBeam2EmitterInstance* TestInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
									check(TestInst);
									static UBOOL s_bSkipBeams = FALSE;
									if (s_bSkipBeams == TRUE)
									{
										this->HandleDynamicDataDeletion(NULL, DynamicEmitterData);
										DynamicEmitterData = NULL;
										ParticleDynamicData->DynamicEmitterDataArray(EmitterIndex) = NULL;
									}
								}
								break;
							case DET_Trail2:
								{
									FParticleTrail2EmitterInstance* TestInst = CastEmitterInstance<FParticleTrail2EmitterInstance>(EmitterInst);
									check(TestInst);
								}
								break;
							default:
								{
									check(0);
								}
								break;
							}
						}
#endif //#if !FINAL_RELEASE
						// Generate the dynamic data for this emitter
						if (DynamicEmitterData == NULL)
						{
							// Create it and update it
							DynamicEmitterData = EmitterInst->GetDynamicData( IsOwnerSelected() );
							if (DynamicEmitterData)
							{
								ParticleDynamicData->DynamicEmitterDataArray(EmitterIndex) = DynamicEmitterData;
								DynamicEmitterData->bValid = TRUE;
							}
						}
						else
						{
							// Just update it
							DynamicEmitterData->bValid = EmitterInst->UpdateDynamicData(DynamicEmitterData, IsOwnerSelected());
						}
					}
					else
					{
						if (DynamicEmitterData)
						{
							// For now, we will delete it to be safe...
							//DynamicEmitterData->bValid = FALSE;
							this->HandleDynamicDataDeletion(NULL, DynamicEmitterData);
							ParticleDynamicData->DynamicEmitterDataArray(EmitterIndex) = NULL;
						}
					}
				}
			}
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
			ParticleDynamicData->DynamicEmitterDataArray.Empty(EmitterInstances.Num());

			for (INT EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
			{
				FDynamicEmitterDataBase* NewDynamicEmitterData = NULL;
				FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
				if (EmitterInst)
				{
					// Generate the dynamic data for this emitter
					NewDynamicEmitterData = EmitterInst->GetDynamicData( IsOwnerSelected() );
					if( NewDynamicEmitterData != NULL )
					{
						NewDynamicEmitterData->bValid = TRUE;
						ParticleDynamicData->DynamicEmitterDataArray.AddItem( NewDynamicEmitterData );
						// Are we current capturing particle state?
						if( ReplayState == PRS_Capturing )
						{
							// Capture replay data for this particle system
							// NOTE: This call should always succeed if GetDynamicData succeeded earlier
							FDynamicEmitterReplayDataBase* NewEmitterReplayData = EmitterInst->GetReplayData();
							check( NewEmitterReplayData != NULL );


							// @todo: We could drastically reduce the size of replays in memory and
							//		on disk by implementing delta compression here.

							// Allocate a new emitter frame
							const INT NewFrameEmitterIndex = NewReplayFrame->Emitters.Num();
							new( NewReplayFrame->Emitters ) FParticleEmitterReplayFrame( EC_EventParm );
							FParticleEmitterReplayFrame* NewEmitterReplayFrame = &NewReplayFrame->Emitters( NewFrameEmitterIndex );

							// Store the replay state for this emitter frame.  Note that this will be
							// deleted when the parent object is garbage collected.
							NewEmitterReplayFrame->EmitterType = NewEmitterReplayData->eEmitterType;
							NewEmitterReplayFrame->OriginalEmitterIndex = EmitterIndex;
							NewEmitterReplayFrame->FrameState = NewEmitterReplayData;
						}
					}
				}
			}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
		}
	}

	return ParticleDynamicData;
}



void UParticleSystemComponent::UpdateDynamicData(FParticleSystemSceneProxy* Proxy)
{
	if (Proxy)
	{
		if (EmitterInstances.Num() > 0)
		{
			INT LiveCount = 0;
			for (INT EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
			{
				FParticleEmitterInstance* EmitInst = EmitterInstances(EmitterIndex);
				if (EmitInst)
				{
					if (EmitInst->ActiveParticles > 0)
					{
						LiveCount++;
					}
				}
			}

			// Only proceed if we have any live particles or if we're actively replaying/capturing
			if( LiveCount > 0 || ReplayState != PRS_Disabled )
			{
				// Create the dynamic data for rendering this particle system
				FParticleDynamicData* ParticleDynamicData = CreateDynamicData();
				if (AccumLODDistanceCheckTime > Template->LODDistanceCheckTime)
				{
					AccumLODDistanceCheckTime	= 0.0f;
					ParticleDynamicData->bNeedsLODDistanceUpdate = TRUE;
					bLODUpdatePending = TRUE;
				}

				// Render the particles
#if PARTICLES_USE_DOUBLE_BUFFERING
#else	//#if PARTICLES_USE_DOUBLE_BUFFERING
#if !FINAL_RELEASE
				//@todo.SAS. Remove thisline  - it is used for debugging purposes...
				Proxy->SetLastDynamicData(Proxy->GetDynamicData());
				//@todo.SAS. END
#endif // !FINAL_RELEASE
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
				Proxy->UpdateData( ParticleDynamicData );
			}
			else
			{
				Proxy->UpdateData(NULL);
			}
		}
		else
		{
			Proxy->UpdateData(NULL);
		}

		// Flip the index...
		GameThreadDynamicDataIndex = 1 - GameThreadDynamicDataIndex;
		check((GameThreadDynamicDataIndex == 0) || (GameThreadDynamicDataIndex == 1));
	}
}

void UParticleSystemComponent::UpdateViewRelevance(FParticleSystemSceneProxy* Proxy)
{
	if ((LODLevel >= 0) && (LODLevel < CachedViewRelevanceFlags.Num()))
	{
		Proxy->UpdateViewRelevance(CachedViewRelevanceFlags(LODLevel));
	}
	else
	if ((LODLevel == -1) && (CachedViewRelevanceFlags.Num() >= 1))
	{
		Proxy->UpdateViewRelevance(CachedViewRelevanceFlags(0));
	}
	else
	{
		FMaterialViewRelevance TempViewRel;
		Proxy->UpdateViewRelevance(TempViewRel);
	}

	bRecacheViewRelevance = FALSE;
}

void UParticleSystemComponent::UpdateLODInformation()
{
	if (GIsGame)
	{
		FParticleSystemSceneProxy* SceneProxy = (FParticleSystemSceneProxy*)Scene_GetProxyFromInfo(SceneInfo);
		if (SceneProxy)
		{
			if (EmitterInstances.Num() > 0)
			{
				BYTE CheckLODMethod = PARTICLESYSTEMLODMETHOD_DirectSet;
				if (bOverrideLODMethod)
				{
					CheckLODMethod = LODMethod;
				}
				else
				{
					if (Template)
					{
						CheckLODMethod = Template->LODMethod;
					}
				}

				if (CheckLODMethod == PARTICLESYSTEMLODMETHOD_Automatic)
				{
					FLOAT PendingDistance = SceneProxy->GetPendingLODDistance();
					if (PendingDistance > 0.0f)
					{
						INT LODIndex = 0;
						for (INT LODDistIndex = 1; LODDistIndex < Template->LODDistances.Num(); LODDistIndex++)
						{
							if (Template->LODDistances(LODDistIndex) > SceneProxy->GetPendingLODDistance())
							{
								break;
							}
							LODIndex = LODDistIndex;
						}

						if (LODIndex != LODLevel)
						{
							SetLODLevel(LODIndex);
						}
					}
				}
			}
		}
	}
	else
	{
		if (LODLevel != EditorLODLevel)
		{
			SetLODLevel(EditorLODLevel);
		}
	}
}

void UParticleSystemComponent::PreEditChange(UProperty* PropertyThatWillChange)
{
	ResetParticles();
	Super::PreEditChange(PropertyThatWillChange);
}

void UParticleSystemComponent::PostEditChange(UProperty* PropertyThatChanged)
{
	bIsViewRelevanceDirty = TRUE;

	InitializeSystem();
	if (bAutoActivate)
	{
		ActivateSystem();
	}
	Super::PostEditChange(PropertyThatChanged);
}

void UParticleSystemComponent::UpdateBounds()
{
	FBox BoundingBox;
	BoundingBox.Init();

	if( Template && Template->bUseFixedRelativeBoundingBox )
	{
		// Use hardcoded relative bounding box from template.
		FVector BoundingBoxOrigin = LocalToWorld.GetOrigin();
		BoundingBox		 = Template->FixedRelativeBoundingBox; 
		BoundingBox.Min += BoundingBoxOrigin;
		BoundingBox.Max += BoundingBoxOrigin;
	}
	else
	{
		BoundingBox += LocalToWorld.GetOrigin();
	
		for (INT i=0; i<EmitterInstances.Num(); i++)
		{
			FParticleEmitterInstance* EmitterInstance = EmitterInstances(i);
			if( EmitterInstance && EmitterInstance->HasActiveParticles() )
			{
				BoundingBox += EmitterInstance->GetBoundingBox();
			}
		}

		// Expand the actual bounding-box slightly so it will be valid longer in the case of expanding particle systems.
		BoundingBox = BoundingBox.ExpandBy(BoundingBox.GetExtent().Size() * 0.1f);
	}

	Bounds = FBoxSphereBounds(BoundingBox);
}

void UParticleSystemComponent::Tick(FLOAT DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_PSysCompTickTime);

#if TRACK_DETAILED_PARTICLE_TICK_STATS
	// Keep track of start time if tracking is enabled.
	DOUBLE StartTime = 0;
	// Create an instance on first use to avoid order of operations with regards to static construction.
	if( !GParticleTickStatManager )
	{
		GParticleTickStatManager = new FParticleTickStatManager();
	}
	// appSeconds can be comparatively costly so we only call it when the stats are enabled.
	if( GParticleTickStatManager->bIsEnabled )
	{
		StartTime = appSeconds();
	}
	// Cache the template as particle finished might reset it.
	UParticleSystem* CachedTemplate = Template;
#endif

#if PARTICLES_USE_DOUBLE_BUFFERING
	// Clear out any pending deletions from previous frames...
	// Need to do this prior to quit-outting of the function to properly cleanup.
	if (DeferredReleaseItems)
	{
		for (INT DeferredIndex = DeferredReleaseItems->DeferredReleases.Num() - 1; DeferredIndex >= 0; DeferredIndex--)
		{
			FParticleDeferredReleaseResources& DeferredRelease = DeferredReleaseItems->DeferredReleases(DeferredIndex);
			if (DeferredRelease.ReleaseFence && (DeferredRelease.ReleaseFence->GetNumPendingFences() == 0))
			{
				delete DeferredRelease.DynamicData;
				DeferredRelease.DynamicData = NULL;
				delete DeferredRelease.EmitterDynamicData;
				DeferredRelease.EmitterDynamicData = NULL;
				delete DeferredRelease.ReleaseFence;
				DeferredRelease.ReleaseFence = NULL;
				DeferredReleaseItems->DeferredReleases.Remove(DeferredIndex);
			}
		}
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

	// Bail out if inactive and not AutoActivate
	if ((bIsActive == FALSE) && (bAutoActivate == FALSE))
	{
		return;
	}

	// Bail out if there is no template, there are no instances, or we're running a dedicated server and we don't update on those
	if ((Template == NULL) || (EmitterInstances.Num() == 0) || ((bUpdateOnDedicatedServer == FALSE) && (GWorld->GetNetMode() == NM_DedicatedServer)))
	{
		return;
	}

	// System settings may have been lowered. Support late deactivation.
	const UBOOL bDetailModeAllowsRendering	= DetailMode <= GSystemSettings.DetailMode;
	if ( bDetailModeAllowsRendering == FALSE )
	{
		if ( bIsActive )
		{
			DeactivateSystem();
			GParticleDataManager.AddParticleSystemComponent(this);
		}
		return;
	}

	// Bail out if MaxSecondsBeforeInactive > 0 and we haven't been rendered the last MaxSecondsBeforeInactive seconds.
	if (bWarmingUp == FALSE)
	{
		AccumLODDistanceCheckTime += DeltaTime;
		if (bLODUpdatePending == TRUE)
		{
			UpdateLODInformation();
			bLODUpdatePending = FALSE;
		}

		const FLOAT MaxSecondsBeforeInactive = Max( SecondsBeforeInactive, Template->SecondsBeforeInactive );
		if( MaxSecondsBeforeInactive > 0 
			&&	AccumTickTime > SecondsBeforeInactive
			&&	GIsGame )
		{
			const FLOAT CurrentTimeSeconds = GWorld->GetTimeSeconds();
			if( CurrentTimeSeconds > (LastRenderTime + MaxSecondsBeforeInactive) )
			{
				bForcedInActive = TRUE;

				SpawnEvents.Empty();
				DeathEvents.Empty();
				CollisionEvents.Empty();
				KismetEvents.Empty();

				return;
			}
		}
	}

	bForcedInActive = FALSE;
	DeltaTime *= GetOwner() ? GetOwner()->CustomTimeDilation : 1.f;

	AccumTickTime += DeltaTime;

	if (bUpdateComponentInTick && GetOwner() != NULL && (NeedsReattach() || NeedsUpdateTransform()))
	{
		if (GetOwner()->Components.ContainsItem(this))
		{
			// directly attached
			UpdateComponent(GetScene(), GetOwner(), GetOwner()->LocalToWorld());
		}
		else if (GWorld->TickGroup != TG_DuringAsyncWork) // we can't safely update everything during async work
		{
			// we have to tell the owner to update everything because we don't know what component we're attached to or how deep it is
			// we could fix this by having a pointer in ActorComponent to the SkeletalMeshComponent that is set in this case
			GetOwner()->ConditionalUpdateComponents(FALSE);
		}
		else
		{
			debugf(NAME_Warning, TEXT("%s (template: %s) needs update but can't be because it's in TG_DuringAsnycWork and indirectly attached!"), *GetName(), *Template->GetPathName());
		}
	}

	if (Template->SystemUpdateMode == EPSUM_FixedTime)
	{
		// Use the fixed delta time!
		DeltaTime = Template->UpdateTime_Delta;
	}

	// Clear out the events.
	SpawnEvents.Empty();
	DeathEvents.Empty();
	CollisionEvents.Empty();

	// Tick Subemitters.
	INT TotalActiveParticles = 0;
	INT EmitterIndex;
	for (EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
	{
		FParticleEmitterInstance* Instance = EmitterInstances(EmitterIndex);

		if (EmitterIndex + 1 < EmitterInstances.Num())
		{
			FParticleEmitterInstance* NextInstance = EmitterInstances(EmitterIndex+1);
			PREFETCH(NextInstance);
		}

		if (Instance && Instance->SpriteTemplate)
		{
			check(Instance->SpriteTemplate->LODLevels.Num() > 0);

			UParticleLODLevel* LODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
			if (LODLevel && LODLevel->bEnabled)
			{
				Instance->Tick(DeltaTime, bSuppressSpawning);
				TotalActiveParticles += Instance->ActiveParticles;
			}
		}
	}

	// Now, process any events that have occurred.
	for (EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
	{
		FParticleEmitterInstance* Instance = EmitterInstances(EmitterIndex);

		if (EmitterIndex + 1 < EmitterInstances.Num())
		{
			FParticleEmitterInstance* NextInstance = EmitterInstances(EmitterIndex+1);
			PREFETCH(NextInstance);
		}

		if (Instance && Instance->SpriteTemplate)
		{
			UParticleLODLevel* LODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
			if (LODLevel && LODLevel->bEnabled)
			{
				Instance->ProcessEvents(DeltaTime, bSuppressSpawning);
			}
		}
	}

	// Clear out the Kismet events, as they should have been processed by now...
	KismetEvents.Empty();

	// Indicate that we have been ticked since being attached.
	bJustAttached = FALSE;

	// If component has just totally finished, call script event.
	const UBOOL bIsCompleted = HasCompleted(); 
	if (bIsCompleted && !bWasCompleted)
	{
		if ( DELEGATE_IS_SET(OnSystemFinished) )
		{
			delegateOnSystemFinished(this);
		}

		if (bIsCachedInPool)
		{
			ConditionalDetach();
			SetHiddenGame(TRUE);
		}
		else
		{
			// When system is done - destroy all subemitters etc. We don't need them any more.
			ResetParticles();
		}	
	}
	bWasCompleted = bIsCompleted;

	// Update bounding box.
	if (!bWarmingUp && !bIsCompleted && !Template->bUseFixedRelativeBoundingBox)
	{
		// Compute the new system bounding box.
		FBox BoundingBox;
		BoundingBox.Init();
	
		// Calculate combined bounding box by combining them from emitter instances.
		BoundingBox += LocalToWorld.GetOrigin();
		for (INT i=0; i<EmitterInstances.Num(); i++)
		{
			FParticleEmitterInstance* Instance = EmitterInstances(i);
			if (Instance && Instance->SpriteTemplate)
			{
				UParticleLODLevel* LODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
	 			if (LODLevel && LODLevel->bEnabled)
				{
					BoundingBox += Instance->GetBoundingBox();
				}
			}
		}

		// Only update the primitive's bounding box in the octree if the system bounding box has gotten larger.
		if(!Bounds.GetBox().IsInside(BoundingBox.Min) || !Bounds.GetBox().IsInside(BoundingBox.Max))
		{
			ConditionalUpdateTransform();
		}
	}

	PartSysVelocity = (LocalToWorld.GetOrigin() - OldPosition) / DeltaTime;
	OldPosition = LocalToWorld.GetOrigin();

	if (bIsViewRelevanceDirty)
	{
		CacheViewRelevanceFlags();
	}

	if (bSkipUpdateDynamicDataDuringTick == FALSE)
	{
		GParticleDataManager.AddParticleSystemComponent(this);
	}

#if TRACK_DETAILED_PARTICLE_TICK_STATS
	if( GParticleTickStatManager->bIsEnabled )
	{
		GParticleTickStatManager->UpdateStats( CachedTemplate, appSeconds() - StartTime, TotalActiveParticles );	
	}
#endif
}

// If particles have not already been initialised (ie. EmitterInstances created) do it now.
void UParticleSystemComponent::InitParticles()
{
	if (IsTemplate() == TRUE)
	{
		return;
	}

	if (Template != NULL)
	{
		WarmupTime = Template->WarmupTime;

		bSkipSpawnCountCheck = Template->bSkipSpawnCountCheck;

		// If nothing is initialized, create EmitterInstances here.
		if (EmitterInstances.Num() == 0)
		{
			SMComponents.Empty();
			SMMaterialInterfaces.Empty();

			const UBOOL bShowInEditor				= !HiddenEditor && (!Owner || !Owner->bHiddenEd);
			const UBOOL bShowInGame					= !HiddenGame && (!Owner || !Owner->bHidden || bIgnoreOwnerHidden || bCastHiddenShadow);
			const UBOOL bDetailModeAllowsRendering	= DetailMode <= GSystemSettings.DetailMode;
			if ( bDetailModeAllowsRendering && ((GIsGame && bShowInGame) || (!GIsGame && bShowInEditor)) )
			{
				EmitterInstances.Empty(Template->Emitters.Num());
				for (INT Idx = 0; Idx < Template->Emitters.Num(); Idx++)
				{
					// Must have a slot for each emitter instance - even if it's NULL.
					// This is so the indexing works correctly.
					UParticleEmitter* Emitter = Template->Emitters(Idx);
					if (Emitter)
					{
						EmitterInstances.AddItem(Emitter->CreateInstance(this));
					}
					else
					{
						INT NewIndex = EmitterInstances.Add();
						EmitterInstances(NewIndex) = NULL;
					}
				}
				bWasCompleted = FALSE;
			}
		}
		else
		{
			// create new instances as needed
			while (EmitterInstances.Num() < Template->Emitters.Num())
			{
				INT					Index	= EmitterInstances.Num();
				UParticleEmitter*	Emitter	= Template->Emitters(Index);
				if (Emitter)
				{
					FParticleEmitterInstance* Instance = Emitter->CreateInstance(this);
					EmitterInstances.AddItem(Instance);
					if (Instance)
					{
						Instance->InitParameters(Emitter, this);
					}
				}
				else
				{
					INT NewIndex = EmitterInstances.Add();
					EmitterInstances(NewIndex) = NULL;
				}
			}

			INT PreferredLODLevel = LODLevel;
			// re-initialize the instances
			for (INT Idx = 0; Idx < EmitterInstances.Num(); Idx++)
			{
				FParticleEmitterInstance* Instance = EmitterInstances(Idx);
				if (Instance) // @FIXME - prevent crash, but is there a logic problem?
				{
					UParticleEmitter* Emitter = NULL;
					
					if (Idx < Template->Emitters.Num())
					{
						Emitter = Template->Emitters(Idx);
					}
					if (Emitter)
					{
						Instance->InitParameters(Emitter, this, FALSE);
						Instance->Init();
						if (PreferredLODLevel >= Emitter->LODLevels.Num())
						{
							PreferredLODLevel = Emitter->LODLevels.Num() - 1;
						}
					}
					else
					{
						// Get rid of the 'phantom' instance
						Instance->RemovedFromScene();
						delete Instance;
						EmitterInstances(Idx) = NULL;
					}
				}
				else
				{
					UParticleEmitter*	Emitter	= Template->Emitters(Idx);
					if (Emitter)
					{
						Instance = Emitter->CreateInstance(this);
						EmitterInstances(Idx) = Instance;
						Instance->InitParameters(Emitter, this, FALSE);
						Instance->Init();
						if (PreferredLODLevel >= Emitter->LODLevels.Num())
						{
							PreferredLODLevel = Emitter->LODLevels.Num() - 1;
						}
					}
					else
					{
						EmitterInstances(Idx) = NULL;
					}
				}
			}

			if (PreferredLODLevel != LODLevel)
			{
				// This should never be higher...
				check(PreferredLODLevel < LODLevel);
				LODLevel = PreferredLODLevel;
			}

			for (INT Idx = 0; Idx < EmitterInstances.Num(); Idx++)
			{
				FParticleEmitterInstance* Instance = EmitterInstances(Idx);
				// set the LOD levels here
				if (Instance)
				{
					Instance->CurrentLODLevelIndex	= LODLevel;
					Instance->CurrentLODLevel		= Instance->SpriteTemplate->LODLevels(Instance->CurrentLODLevelIndex);
				}
			}
		}
	}
}

void UParticleSystemComponent::ResetParticles(UBOOL bEmptyInstances)
{
	// Remove instances from scene.
	for( INT InstanceIndex=0; InstanceIndex<EmitterInstances.Num(); InstanceIndex++ )
	{
		FParticleEmitterInstance* EmitterInstance = EmitterInstances(InstanceIndex);
		if( EmitterInstance )
		{
			EmitterInstance->RemovedFromScene();
			if (!(!GIsGame || bEmptyInstances))
			{
				EmitterInstance->SpriteTemplate	= NULL;
				EmitterInstance->Component	= NULL;
			}
		}
	}

	// Set the system as deactive
	bIsActive	= FALSE;

	// Remove instances if we're not running gameplay.
	if (!GIsGame || bEmptyInstances)
	{
		if (GIsGame)
		{
			AEmitterPool* EmitterPool = NULL;
			if (GWorld && GWorld->GetWorldInfo())
			{
				EmitterPool = GWorld->GetWorldInfo()->MyEmitterPool;
				if (EmitterPool)
				{
					EmitterPool->FreeStaticMeshComponents(this);
				}
			}
		}

#if PARTICLES_USE_DOUBLE_BUFFERING
		// Flushing the dynamic data to be safe here...
		if (DynamicDataBuffers[0])
		{
			DynamicDataBuffers[0]->bForceTemplateReset = TRUE;
		}
		if (DynamicDataBuffers[1])
		{
			DynamicDataBuffers[1]->bForceTemplateReset = TRUE;
		}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING

		SMComponents.Empty();
		SMMaterialInterfaces.Empty();
		for (INT EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
		{
			FParticleEmitterInstance* EmitInst = EmitterInstances(EmitterIndex);
			if (EmitInst)
			{
				delete EmitInst;
				EmitterInstances(EmitterIndex) = NULL;
			}
		}
		EmitterInstances.Empty();
	}
	else
	{
		for (INT EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
		{
			FParticleEmitterInstance* EmitInst = EmitterInstances(EmitterIndex);
			if (EmitInst)
			{
				EmitInst->Rewind();
			}
		}
	}
}

void UParticleSystemComponent::ResetBurstLists()
{
	for (INT i=0; i<EmitterInstances.Num(); i++)
	{
		if (EmitterInstances(i))
		{
			EmitterInstances(i)->ResetBurstList();
		}
	}
}

void UParticleSystemComponent::SetTemplate(class UParticleSystem* NewTemplate)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticleSetTemplateTime);

	if( GIsAllowingParticles || GIsEditor ) 
	{
		bIsViewRelevanceDirty = TRUE;

		UBOOL bIsTemplate = IsTemplate();
		// duplicated in ActivateSystem
		// check to see if we need to update the component
		if (bIsTemplate == FALSE && NewTemplate )
		{
			if (Owner != NULL)
			{
				UpdateComponent(GWorld->Scene,Owner,Owner->LocalToWorld());
			}
		}
		bWasCompleted = FALSE;
		// remember if we were active and therefore should restart after setting up the new template
		UBOOL bWasActive = bIsActive; 
		UBOOL bResetInstances = FALSE;
		if (NewTemplate != Template)
		{
			bResetInstances = TRUE;
		}
		if (bIsTemplate == FALSE)
		{
			ResetParticles(bResetInstances);
		}

		Template = NewTemplate;
		if (Template)
		{
			WarmupTime = Template->WarmupTime;
			bAcceptsLights = Template->bLit;
		}
		else
		{
			WarmupTime = 0.0f;
			bAcceptsLights = FALSE;
		}

		if( NewTemplate )
		{
			if ((bAutoActivate || bWasActive) && (bIsTemplate == FALSE))
			{
				ActivateSystem();
			}
			else
			{
				InitializeSystem();
			}
			if ((SceneInfo == NULL) || bResetInstances)
			{
				BeginDeferredReattach();
			}
		}
	}
	else
	{
		Template = NULL;
	}
}

void UParticleSystemComponent::ActivateSystem(UBOOL bFlagAsJustAttached)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticleActivateTime);

	if (IsTemplate() == TRUE)
	{
		return;
	}

#if defined(_LOG_POTENTIAL_ACTIVATION_ERRORS_)
if (Template)
{
	if (EmitterInstances.Num() > 0)
	{
		INT LiveCount = 0;

		for (INT EmitterIndex =0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
		{
			FParticleEmitterInstance* EmitInst = EmitterInstances(EmitterIndex);
			if (EmitInst)
			{
				LiveCount += EmitInst->ActiveParticles;
			}
		}

		if (LiveCount > 0)
		{
			debugf(TEXT("ActivateSystem called on PSysComp w/ live particles - %5d, %s"),
				LiveCount, *(Template->GetFullName()));
		}
	}
}
#endif  //#if defined(_LOG_POTENTIAL_ACTIVATION_ERRORS_)

	// System settings may have been lowered. Support late deactivation.
	const UBOOL bDetailModeAllowsRendering	= DetailMode <= GSystemSettings.DetailMode;

	if( GIsAllowingParticles && bDetailModeAllowsRendering )
	{
		if (bFlagAsJustAttached)
		{
			bJustAttached = TRUE;
		}

		// Stop suppressing particle spawning.
		bSuppressSpawning = FALSE;
		
		// Set the system as active
		UBOOL bNeedToUpdateTransform = bWasDeactivated;
		bWasCompleted = FALSE;
		bWasDeactivated = FALSE;
		bIsActive = TRUE;

		if (SceneInfo == NULL)
		{
			BeginDeferredUpdateTransform();
		}

		// if no instances, or recycling
		if (EmitterInstances.Num() == 0 || GIsGame)
		{
			InitializeSystem();
		}
		else
		{
			// If currently running, re-activating rewinds the emitter to the start. Existing particles should stick around.
			for (INT i=0; i<EmitterInstances.Num(); i++)
			{
				if (EmitterInstances(i))
				{
					EmitterInstances(i)->Rewind();
				}
			}
		}

		// duplicated in SetTemplate
		// check to see if we need to update the component
		if (Owner != NULL)
		{
			if( bNeedToUpdateTransform )
			{
				DirtyTransform();
			}
			UpdateComponent(GWorld->Scene,Owner,Owner->LocalToWorld());
		}
		else if (bNeedToUpdateTransform)
		{
			ConditionalUpdateTransform();
		}

		if (WarmupTime != 0.0f)
		{
			UBOOL bSaveSkipUpdate = bSkipUpdateDynamicDataDuringTick;
			bSkipUpdateDynamicDataDuringTick = TRUE;
			bWarmingUp = TRUE;
			ResetBurstLists();

			FLOAT WarmupElapsed = 0.f;
			FLOAT WarmupTimestep = 0.032f;

			while (WarmupElapsed < WarmupTime)
			{
				Tick(WarmupTimestep);
				WarmupElapsed += WarmupTimestep;
			}

			bWarmingUp = FALSE;
			WarmupTime = 0.0f;
			bSkipUpdateDynamicDataDuringTick = bSaveSkipUpdate;
		}
		AccumTickTime = 0.0;
	}

	GParticleDataManager.AddParticleSystemComponent(this);

	LastRenderTime = GWorld->GetTimeSeconds();
}

/**
 *	DeactivateSystem
 *	Called to deactivate the particle system.
 *
 *	@param	bResetParticles		If TRUE, call ResetParticles on each emitter.
 */
void UParticleSystemComponent::DeactivateSystem()
{
	if (IsTemplate() == TRUE)
	{
		return;
	}

	bSuppressSpawning = TRUE;
	bWasDeactivated = TRUE;

	for (INT i = 0; i < EmitterInstances.Num(); i++)
	{
		FParticleEmitterInstance*	Instance = EmitterInstances(i);
		if (Instance)
		{
			if (Instance->KillOnDeactivate)
			{
				//debugf(TEXT("%s killed on deactivate"),EmitterInstances(i)->GetName());
				Instance->RemovedFromScene();
				delete Instance;
				EmitterInstances(i) = NULL;
			}
		}
	}

	LastRenderTime = GWorld->GetTimeSeconds();
}

/** calls ActivateSystem() or DeactivateSystem() only if the component is not already activated/deactivated
 * necessary because ActivateSystem() resets already active emitters so it shouldn't be called multiple times on looping effects
 * @param bNowActive - whether the system should be active
 */
void UParticleSystemComponent::SetActive(UBOOL bNowActive)
{
	if (bNowActive)
	{
		if (!bIsActive || (bWasDeactivated || bWasCompleted))
		{
			ActivateSystem();
		}
	}
	else if (bIsActive && !(bWasDeactivated || bWasCompleted))
	{
		DeactivateSystem();
	}
}

/** stops the emitter, detaches the component, and resets the component's properties to the values of its template */
void UParticleSystemComponent::ResetToDefaults()
{
	if (!IsTemplate())
	{
		// make sure we're fully stopped and detached
		DeactivateSystem();
		SetTemplate(NULL);
		DetachFromAny();

		UParticleSystemComponent* Default = GetArchetype<UParticleSystemComponent>();

		// copy all non-native, non-duplicatetransient, non-Component properties we have from all classes up to and including UActorComponent
		for (UProperty* Property = GetClass()->PropertyLink; Property != NULL; Property = Property->PropertyLinkNext)
		{
			if ( !(Property->PropertyFlags & CPF_Native) && !(Property->PropertyFlags & CPF_DuplicateTransient) && !(Property->PropertyFlags & CPF_Component) &&
				Property->GetOwnerClass()->IsChildOf(UActorComponent::StaticClass()) )
			{
				Property->CopyCompleteValue((BYTE*)this + Property->Offset, (BYTE*)Default + Property->Offset, NULL, this);
			}
		}
	}
}

void UParticleSystemComponent::UpdateInstances()
{
	SCOPE_CYCLE_COUNTER(STAT_ParticleUpdateInstancesTime);

	ResetParticles();

	InitializeSystem();
	if (bAutoActivate)
	{
		ActivateSystem();
	}

	if (Template && Template->bUseFixedRelativeBoundingBox)
	{
		ConditionalUpdateTransform();
	}
}

UBOOL UParticleSystemComponent::HasCompleted()
{
	UBOOL bHasCompleted = TRUE;

	// If we're currently capturing or replaying captured frames, then we'll stay active for that
	if( ReplayState != PRS_Disabled )
	{
		// While capturing, we want to stay active so that we'll just record empty frame data for
		// completed particle systems.  While replaying, we never want our particles/meshes removed from
		// the scene, so we'll force the system to stay alive!
		return FALSE;
	}

	for (INT i=0; i<EmitterInstances.Num(); i++)
	{
		FParticleEmitterInstance* Instance = EmitterInstances(i);

		if (Instance && Instance->CurrentLODLevel)
		{
			if (Instance->CurrentLODLevel->RequiredModule->EmitterLoops > 0)
			{
				if (Instance->CurrentLODLevel->bEnabled)
				{
					if (bWasDeactivated && bSuppressSpawning)
					{
						if (Instance->ActiveParticles != 0)
						{
							bHasCompleted = FALSE;
						}
					}
					else
					{
						if (Instance->HasCompleted())
						{
							if (Instance->bKillOnCompleted)
							{
								Instance->RemovedFromScene();
								delete Instance;
								EmitterInstances(i) = NULL;
							}
						}
						else
						{
							bHasCompleted = FALSE;
						}
					}
				}
			}
			else
			{
				if (bWasDeactivated)
				{
					if (Instance->ActiveParticles != 0)
					{
						bHasCompleted = FALSE;
					}
				}
				else
				{
					bHasCompleted = FALSE;
				}
			}
		}
	}

	return bHasCompleted;
}

void UParticleSystemComponent::InitializeSystem()
{
	SCOPE_CYCLE_COUNTER(STAT_ParticleInitializeTime);

	// System settings may have been lowered. Support late deactivation.
	const UBOOL bDetailModeAllowsRendering	= DetailMode <= GSystemSettings.DetailMode;

	if( GIsAllowingParticles && bDetailModeAllowsRendering )
	{
		if (IsTemplate() == TRUE)
		{
			return;
		}

		// Allocate the emitter instances and particle data
		InitParticles();
		if (IsAttached())
		{
			AccumTickTime = 0.0;
			if ((bIsActive == FALSE) && (bAutoActivate == TRUE) && (bWasDeactivated == FALSE))
			{
				SetActive(TRUE);
			}
		}
	}
}

/**
 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
 * you have a component of interest but what you really want is some characteristic that you can use to track
 * down where it came from.  
 *
 */
FString UParticleSystemComponent::GetDetailedInfoInternal() const
{
	FString Result;  

	if( Template != NULL )
	{
		Result = Template->GetPathName( NULL );
	}
	else
	{
		Result = TEXT("No_ParticleSystem");
	}

	return Result;  
}



/**
 *	Cache the view-relevance for each emitter at each LOD level.
 *
 *	@param	NewTemplate		The UParticleSystem* to use as the template.
 *							If NULL, use the currently set template.
 */
void UParticleSystemComponent::CacheViewRelevanceFlags(class UParticleSystem* NewTemplate)
{
	if (NewTemplate && (NewTemplate != Template))
	{
		bIsViewRelevanceDirty = TRUE;
		CachedViewRelevanceFlags.Empty();
	}

	if (bIsViewRelevanceDirty)
	{
		UParticleSystem* TemplateToCache = Template;
		if (NewTemplate)
		{
			TemplateToCache = NewTemplate;
		}

		if (TemplateToCache)
		{
			for (INT EmitterIndex = 0; EmitterIndex < TemplateToCache->Emitters.Num(); EmitterIndex++)
			{
				UParticleSpriteEmitter* Emitter = Cast<UParticleSpriteEmitter>(TemplateToCache->Emitters(EmitterIndex));
				if (Emitter == NULL)
				{
					// Handle possible empty slots in the emitter array.
					continue;
				}
				FParticleEmitterInstance* EmitterInst = NULL;
				if (EmitterIndex < EmitterInstances.Num())
				{
					EmitterInst = EmitterInstances(EmitterIndex);
				}

				FParticleMeshEmitterInstance* MeshEmitInst = CastEmitterInstance<FParticleMeshEmitterInstance>(EmitterInst);

				for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
				{
					UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);

					// Prime the array
					// This code assumes that the particle system emitters all have the same number of LODLevels. 
					if (LODIndex >= CachedViewRelevanceFlags.Num())
					{
						CachedViewRelevanceFlags.AddZeroed(1);
					}
					FMaterialViewRelevance& LODViewRel = CachedViewRelevanceFlags(LODIndex);
					check(LODLevel->RequiredModule);

					FMaterialViewRelevance LocalMaterialViewRelevance;
					if (LODLevel->bEnabled == TRUE)
					{
						UMaterialInterface* MaterialInst = NULL;
						check(Emitter->LODLevels.Num() >= 1);
						UParticleLODLevel* ZeroLODLevel = Emitter->LODLevels(0);
						if (ZeroLODLevel && ZeroLODLevel->TypeDataModule)
						{
							UParticleModuleTypeDataMesh* MeshTD = Cast<UParticleModuleTypeDataMesh>(ZeroLODLevel->TypeDataModule);
							if (MeshTD && MeshTD->Mesh)
							{
								const FStaticMeshRenderData& LODModel = MeshTD->Mesh->LODModels(0);
								// Gather the materials applied to the LOD.
								for (INT ElementIndex = 0; ElementIndex < LODModel.Elements.Num(); ElementIndex++)
								{
									if (MeshEmitInst)
									{
										if (ElementIndex < MeshEmitInst->CurrentMaterials.Num())
										{
											MaterialInst = MeshEmitInst->CurrentMaterials(ElementIndex);
										}

										// See if there is a mesh material module.
										if (MaterialInst == NULL)
										{
											for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
											{
												UParticleModuleMeshMaterial* MeshMatModule = Cast<UParticleModuleMeshMaterial>(LODLevel->Modules(ModuleIndex));
												if (MeshMatModule && MeshMatModule->bEnabled)
												{
													if (ElementIndex < MeshMatModule->MeshMaterials.Num())
													{
														MaterialInst = MeshMatModule->MeshMaterials(ElementIndex);
														break;
													}
												}
											}
										}

										if (MaterialInst == NULL)
										{
											// Are we overriding the material w/ the sprite one?
											if (MeshTD->bOverrideMaterial == TRUE)
											{
												MaterialInst = LODLevel->RequiredModule->Material;
											}
										}

										if (MaterialInst == NULL)
										{
											if ((MeshEmitInst->MeshComponentIndex > -1) && (MeshEmitInst->MeshComponentIndex < SMComponents.Num()))
											{
												UStaticMeshComponent* MeshComponent = SMComponents(MeshEmitInst->MeshComponentIndex);
												if (MeshComponent)
												{
													// The emitter instance Materials array will be filled in with entries from the 
													// MeshMaterial module, if present.
													if (MeshComponent->Materials.Num() > ElementIndex)
													{
														MaterialInst = MeshComponent->Materials(ElementIndex);
													}
												}
											}
										}

										if (MaterialInst == NULL)
										{
											// Is the emitter instance CurrentMaterial set to one?
											MaterialInst = MeshEmitInst->CurrentMaterial;
										}
									}

									if (MaterialInst == NULL)
									{
										// Grab the material assigned to the static mesh itself
										if (ElementIndex < MeshTD->Mesh->LODModels(0).Elements.Num())
										{
											FStaticMeshElement&	Element = MeshTD->Mesh->LODModels(0).Elements(ElementIndex);
											if (Element.Material)
											{
												MaterialInst = Element.Material;
											}
										}
									}

									// Use the default material...
									if (MaterialInst == NULL)
									{
										MaterialInst = GEngine->DefaultMaterial;
									}

									// Let the last one fall through...
									if ((MaterialInst) &&
										(ElementIndex < (LODModel.Elements.Num() - 1)))
									{
										LODViewRel |= MaterialInst->GetViewRelevance();
									}
								}
							}
						}

						// These will catch the sprite cases...
						if (MaterialInst == NULL)
						{
							if (EmitterInst)
							{
								MaterialInst = EmitterInst->CurrentMaterial;
							}
						}
						if (MaterialInst == NULL)
						{
							MaterialInst = LODLevel->RequiredModule->Material;
						}
						// Use the default material...
						if (MaterialInst == NULL)
						{
							MaterialInst = GEngine->DefaultMaterial;
						}

						// Or in the view relevance
						if(MaterialInst)
						{
							LODViewRel |= MaterialInst->GetViewRelevance();
						}
					}

					// Don't allow the emitter to render distortion if set by system settings
					if (GSystemSettings.bAllowParticleDistortionDropping
						&& GWorld
						// And WorldInfo is indicating that high detail should be dropped
						&& GWorld->GetWorldInfo()->bDropDetail)
					{
						LODViewRel.bDistortion = FALSE;
					}
				}
			}
		}
		bIsViewRelevanceDirty = FALSE;
		bRecacheViewRelevance = TRUE;
	}
}

/**
 * SetKillOnDeactivate is used to set the KillOnDeactivate flag. If true, when
 * the particle system is deactivated, it will immediately kill the emitter
 * instance. If false, the emitter instance live particles will complete their
 * lifetime.
 *
 * Set this to true for cached ParticleSystems
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	bKill				value to set KillOnDeactivate to
 */
void UParticleSystemComponent::SetKillOnDeactivate(INT EmitterIndex,UBOOL bKill)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			EmitterInst->SetKillOnDeactivate(bKill);
		}
	}
}

/**
 * SetKillOnDeactivate is used to set the KillOnCompleted( flag. If true, when
 * the particle system is completed, it will immediately kill the emitter
 * instance.
 *
 * Set this to true for cached ParticleSystems
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	bKill				The value to set it to
 **/
void UParticleSystemComponent::SetKillOnCompleted(INT EmitterIndex,UBOOL bKill)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			EmitterInst->SetKillOnCompleted(bKill);
		}
	}
}

/**
 * Rewind emitter instances.
 **/
void UParticleSystemComponent::RewindEmitterInstance(INT EmitterIndex)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			EmitterInst->Rewind();
		}
	}
}

void UParticleSystemComponent::RewindEmitterInstances()
{
	for (INT EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			EmitterInst->Rewind();
		}
	}
}

/**
 *	Set the beam type
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewMethod			The new method/type of beam to generate
 */
void UParticleSystemComponent::SetBeamType(INT EmitterIndex,INT NewMethod)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetBeamType(NewMethod);
			}
		}
	}
}

/**
 *	Set the beam tessellation factor
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewFactor			The value to set it to
 */
void UParticleSystemComponent::SetBeamTessellationFactor(INT EmitterIndex,FLOAT NewFactor)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetTessellationFactor(NewFactor);
			}
		}
	}
}

/**
 *	Set the beam end point
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewEndPoint			The value to set it to
 */
void UParticleSystemComponent::SetBeamEndPoint(INT EmitterIndex,FVector NewEndPoint)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetEndPoint(NewEndPoint);
			}
		}
	}
}

/**
 *	Set the beam distance
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	Distance			The value to set it to
 */
void UParticleSystemComponent::SetBeamDistance(INT EmitterIndex,FLOAT Distance)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetDistance(Distance);
			}
		}
	}
}

/**
 *	Set the beam source point
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewSourcePoint		The value to set it to
 *	@param	SourceIndex			Which beam within the emitter to set it on
 */
void UParticleSystemComponent::SetBeamSourcePoint(INT EmitterIndex,FVector NewSourcePoint,INT SourceIndex)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetSourcePoint(NewSourcePoint, SourceIndex);
			}
		}
	}
}

/**
 *	Set the beam source tangent
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewTangentPoint		The value to set it to
 *	@param	SourceIndex			Which beam within the emitter to set it on
 */
void UParticleSystemComponent::SetBeamSourceTangent(INT EmitterIndex,FVector NewTangentPoint,INT SourceIndex)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetSourceTangent(NewTangentPoint, SourceIndex);
			}
		}
	}
}

/**
 *	Set the beam source strength
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewSourceStrength	The value to set it to
 *	@param	SourceIndex			Which beam within the emitter to set it on
 */
void UParticleSystemComponent::SetBeamSourceStrength(INT EmitterIndex,FLOAT NewSourceStrength,INT SourceIndex)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetSourceStrength(NewSourceStrength, SourceIndex);
			}
		}
	}
}

/**
 *	Set the beam target point
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewTargetPoint		The value to set it to
 *	@param	TargetIndex			Which beam within the emitter to set it on
 */
void UParticleSystemComponent::SetBeamTargetPoint(INT EmitterIndex,FVector NewTargetPoint,INT TargetIndex)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetTargetPoint(NewTargetPoint, TargetIndex);
			}
		}
	}
}

/**
 *	Set the beam target tangent
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewTangentPoint		The value to set it to
 *	@param	TargetIndex			Which beam within the emitter to set it on
 */
void UParticleSystemComponent::SetBeamTargetTangent(INT EmitterIndex,FVector NewTangentPoint,INT TargetIndex)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetTargetTangent(NewTangentPoint, TargetIndex);
			}
		}
	}
}

/**
 *	Set the beam target strength
 *
 *	@param	EmitterIndex		The index of the emitter to set it on
 *	@param	NewTargetStrength	The value to set it to
 *	@param	TargetIndex			Which beam within the emitter to set it on
 */
void UParticleSystemComponent::SetBeamTargetStrength(INT EmitterIndex,FLOAT NewTargetStrength,INT TargetIndex)
{
	if (EmitterInstances.Num() == 0)
	{
		return;
	}

	if ((EmitterIndex >= 0) && EmitterIndex < EmitterInstances.Num())
	{
		FParticleEmitterInstance* EmitterInst = EmitterInstances(EmitterIndex);
		if (EmitterInst)
		{
			FParticleBeam2EmitterInstance* BeamInst = CastEmitterInstance<FParticleBeam2EmitterInstance>(EmitterInst);
			if (BeamInst)
			{
				BeamInst->SetTargetStrength(NewTargetStrength, TargetIndex);
			}
		}
	}
}

/** Set the LOD level of the particle system									*/
void UParticleSystemComponent::SetLODLevel(int InLODLevel)
{
	if (Template == NULL)
	{
		return;
	}

	INT NewLODLevel = Clamp(InLODLevel + GSystemSettings.ParticleLODBias,0,Template->GetLODLevelCount()-1);
	if (LODLevel != NewLODLevel)
	{
		bRecacheViewRelevance = TRUE;

		LODLevel = NewLODLevel;

		for (INT i=0; i<EmitterInstances.Num(); i++)
		{
			FParticleEmitterInstance* Instance = EmitterInstances(i);
			if (Instance)
			{
				if (Instance->SpriteTemplate != NULL)
				{
					Instance->CurrentLODLevelIndex	= LODLevel;
					// check to make certain the data in the content actually represents what we are being asked to render
					if( Instance->SpriteTemplate->LODLevels.Num() > Instance->CurrentLODLevelIndex )
					{
						Instance->CurrentLODLevel		= Instance->SpriteTemplate->LODLevels(Instance->CurrentLODLevelIndex);
					}
					// set to the LOD which is guaranteed to exist
					else
					{
						Instance->CurrentLODLevelIndex = 0;
						Instance->CurrentLODLevel = Instance->SpriteTemplate->LODLevels(Instance->CurrentLODLevelIndex);
					}

					check(Instance->CurrentLODLevel);
					check(Instance->CurrentLODLevel->RequiredModule);
					Instance->bKillOnCompleted = Instance->CurrentLODLevel->RequiredModule->bKillOnCompleted;
					Instance->KillOnDeactivate = Instance->CurrentLODLevel->RequiredModule->bKillOnDeactivate;

					// Check for bursts that should have been fired already...
					UParticleModuleSpawn* SpawnModule = Instance->CurrentLODLevel->SpawnModule;
					FLODBurstFired* BurstFired = NULL;

					if (Instance->CurrentLODLevelIndex < Instance->BurstFired.Num())
					{
						BurstFired = &(Instance->BurstFired(Instance->CurrentLODLevelIndex));
					}
					else
					{
						// This should not happen, but catch it just in case...
						Instance->BurstFired.AddZeroed(Instance->CurrentLODLevelIndex - Instance->BurstFired.Num());
						BurstFired = &(Instance->BurstFired(Instance->CurrentLODLevelIndex));
					}

					if (BurstFired->BurstFired.Num() < SpawnModule->BurstList.Num())
					{
						BurstFired->BurstFired.AddZeroed(SpawnModule->BurstList.Num() - BurstFired->BurstFired.Num());
					}

					for (INT BurstIndex = 0; BurstIndex < SpawnModule->BurstList.Num(); BurstIndex++)
					{
						if (SpawnModule->BurstList(BurstIndex).Time < Instance->EmitterTime)
						{
							BurstFired->BurstFired(BurstIndex) = TRUE;
						}
					}
				}
				else
				{
					// This is a legitimate case when PSysComponents are cached...
					// However, with the addition of the bIsActive flag to that class, this should 
					// never be called when the component has not had it's instances initialized/activated.
#if defined(_PSYSCOMP_DEBUG_INVALID_EMITTER_INSTANCE_TEMPLATES_)
					// need better debugging here
					warnf(TEXT("Template of emitter instance %d (%s) a ParticleSystemComponent (%s) was NULL: %s" ), 
						i, *Instance->GetName(), *Template->GetName(), *this->GetFullName());
#endif	//#if defined(_PSYSCOMP_DEBUG_INVALID_EMITTER_INSTANCE_TEMPLATES_)
				}
			}
		}
	}
}

/** Set the editor LOD level of the particle system								*/
void UParticleSystemComponent::SetEditorLODLevel(INT InLODLevel)
{
	if (InLODLevel < 0)
	{
		InLODLevel	= 0;
	}

	EditorLODLevel	= InLODLevel;
}

/** Get the LOD level of the particle system									*/
INT UParticleSystemComponent::GetLODLevel()
{
	return LODLevel;
}

/** Get the editor LOD level of the particle system								*/
INT UParticleSystemComponent::GetEditorLODLevel()
{
	return EditorLODLevel;
}

/**
 *	Determine the appropriate LOD level to utilize
 */
INT UParticleSystemComponent::DetermineLODLevel(const FSceneView* View)
{
	INT	LODIndex = -1;

	BYTE CheckLODMethod = PARTICLESYSTEMLODMETHOD_DirectSet;
	if (bOverrideLODMethod)
	{
		CheckLODMethod = LODMethod;
	}
	else
	{
		if (Template)
		{
			CheckLODMethod = Template->LODMethod;
		}
	}

	if (CheckLODMethod == PARTICLESYSTEMLODMETHOD_Automatic)
	{
		// Default to the highest LOD level
		LODIndex = 0;

		FVector	CameraPosition	= View->ViewOrigin;
		FVector	CompPosition	= LocalToWorld.GetOrigin();
		FVector	DistDiff		= CompPosition - CameraPosition;
		FLOAT	Distance		= DistDiff.Size();
		//debugf(TEXT("Perform LOD Determination Check - Dist = %8.5f, PSys %s"), Distance, *GetFullName());

		for (INT LODDistIndex = 1; LODDistIndex < Template->LODDistances.Num(); LODDistIndex++)
		{
			if (Template->LODDistances(LODDistIndex) > Distance)
			{
				break;
			}
			LODIndex = LODDistIndex;
		}
	}

	return LODIndex;
}

/** 
 *	Set a named float instance parameter on this ParticleSystemComponent. 
 *	Updates the parameter if it already exists, or creates a new entry if not. 
 */
void UParticleSystemComponent::SetFloatParameter(FName Name, FLOAT Param)
{
	if(Name == NAME_None)
	{
		return;
	}

	// First see if an entry for this name already exists
	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& P = InstanceParameters(i);
		if (P.Name == Name && P.ParamType == PSPT_Scalar)
		{
			P.Scalar = Param;
			return;
		}
	}

	// We didn't find one, so create a new one.
	INT NewParamIndex = InstanceParameters.AddZeroed();
	InstanceParameters(NewParamIndex).Name = Name;
	InstanceParameters(NewParamIndex).ParamType = PSPT_Scalar;
	InstanceParameters(NewParamIndex).Scalar = Param;
}

/** 
 *	Set a named vector instance parameter on this ParticleSystemComponent. 
 *	Updates the parameter if it already exists, or creates a new entry if not. 
 */
void UParticleSystemComponent::SetVectorParameter(FName Name, FVector Param)
{
	if(Name == NAME_None)
	{
		return;
	}

	// First see if an entry for this name already exists
	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& P = InstanceParameters(i);
		if (P.Name == Name && P.ParamType == PSPT_Vector)
		{
			P.Vector = Param;
			return;
		}
	}

	// We didn't find one, so create a new one.
	INT NewParamIndex = InstanceParameters.AddZeroed();
	InstanceParameters(NewParamIndex).Name = Name;
	InstanceParameters(NewParamIndex).ParamType = PSPT_Vector;
	InstanceParameters(NewParamIndex).Vector = Param;
}

/** 
 *	Set a named color instance parameter on this ParticleSystemComponent. 
 *	Updates the parameter if it already exists, or creates a new entry if not. 
 */
void UParticleSystemComponent::SetColorParameter(FName Name, FColor Param)
{
	if(Name == NAME_None)
	{
		return;
	}

	// First see if an entry for this name already exists
	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& P = InstanceParameters(i);
		if (P.Name == Name && P.ParamType == PSPT_Color)
		{
			P.Color = Param;
			return;
		}
	}

	// We didn't find one, so create a new one.
	INT NewParamIndex = InstanceParameters.AddZeroed();
	InstanceParameters(NewParamIndex).Name = Name;
	InstanceParameters(NewParamIndex).ParamType = PSPT_Color;
	InstanceParameters(NewParamIndex).Color = Param;
}

/** 
 *	Set a named actor instance parameter on this ParticleSystemComponent. 
 *	Updates the parameter if it already exists, or creates a new entry if not. 
 */
void UParticleSystemComponent::SetActorParameter(FName Name, AActor* Param)
{
	if(Name == NAME_None)
	{
		return;
	}

	// First see if an entry for this name already exists
	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& P = InstanceParameters(i);
		if (P.Name == Name && P.ParamType == PSPT_Actor)
		{
			P.Actor = Param;
			return;
		}
	}

	// We didn't find one, so create a new one.
	INT NewParamIndex = InstanceParameters.AddZeroed();
	InstanceParameters(NewParamIndex).Name = Name;
	InstanceParameters(NewParamIndex).ParamType = PSPT_Actor;
	InstanceParameters(NewParamIndex).Actor = Param;
}

/** 
 *	Set a named material instance parameter on this ParticleSystemComponent. 
 *	Updates the parameter if it already exists, or creates a new entry if not. 
 */
void UParticleSystemComponent::SetMaterialParameter(FName Name, UMaterialInterface* Param)
{
	if(Name == NAME_None)
	{
		return;
	}

	// First see if an entry for this name already exists
	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& P = InstanceParameters(i);
		if (P.Name == Name && P.ParamType == PSPT_Material)
		{
			bIsViewRelevanceDirty = (P.Material != Param) ? TRUE : FALSE;
			P.Material = Param;
			return;
		}
	}

	// We didn't find one, so create a new one.
	INT NewParamIndex = InstanceParameters.AddZeroed();
	InstanceParameters(NewParamIndex).Name = Name;
	InstanceParameters(NewParamIndex).ParamType = PSPT_Material;
	bIsViewRelevanceDirty = (InstanceParameters(NewParamIndex).Material != Param) ? TRUE : FALSE;
	InstanceParameters(NewParamIndex).Material = Param;
}

/**
 *	Retrieve the Float parameter value for the given name.
 *
 *	@param	InName		Name of the parameter
 *	@param	OutFloat	The value of the parameter found
 *
 *	@return	TRUE		Parameter was found - OutFloat is valid
 *			FALSE		Parameter was not found - OutFloat is invalid
 */
UBOOL UParticleSystemComponent::GetFloatParameter(const FName InName,FLOAT& OutFloat)
{
	// Always fail if we pass in no name.
	if(InName == NAME_None)
	{
		return FALSE;
	}

	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& Param = InstanceParameters(i);
		if (Param.Name == InName && Param.ParamType == PSPT_Scalar)
		{
			OutFloat = Param.Scalar;
			return TRUE;
		}
	}

	return FALSE;
}

/**
 *	Retrieve the Vector parameter value for the given name.
 *
 *	@param	InName		Name of the parameter
 *	@param	OutVector	The value of the parameter found
 *
 *	@return	TRUE		Parameter was found - OutVector is valid
 *			FALSE		Parameter was not found - OutVector is invalid
 */
UBOOL UParticleSystemComponent::GetVectorParameter(const FName InName,FVector& OutVector)
{
	// Always fail if we pass in no name.
	if(InName == NAME_None)
	{
		return FALSE;
	}

	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& Param = InstanceParameters(i);
		if (Param.Name == InName && Param.ParamType == PSPT_Vector)
		{
			OutVector = Param.Vector;
			return TRUE;
		}
	}

	return FALSE;
}

/**
 *	Retrieve the Color parameter value for the given name.
 *
 *	@param	InName		Name of the parameter
 *	@param	OutColor	The value of the parameter found
 *
 *	@return	TRUE		Parameter was found - OutColor is valid
 *			FALSE		Parameter was not found - OutColor is invalid
 */
UBOOL UParticleSystemComponent::GetColorParameter(const FName InName,FColor& OutColor)
{
	// Always fail if we pass in no name.
	if(InName == NAME_None)
	{
		return FALSE;
	}

	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& Param = InstanceParameters(i);
		if (Param.Name == InName && Param.ParamType == PSPT_Color)
		{
			OutColor = Param.Color;
			return TRUE;
		}
	}

	return FALSE;
}

/**
 *	Retrieve the Actor parameter value for the given name.
 *
 *	@param	InName		Name of the parameter
 *	@param	OutActor	The value of the parameter found
 *
 *	@return	TRUE		Parameter was found - OutActor is valid
 *			FALSE		Parameter was not found - OutActor is invalid
 */
UBOOL UParticleSystemComponent::GetActorParameter(const FName InName,class AActor*& OutActor)
{
	// Always fail if we pass in no name.
	if (InName == NAME_None)
	{
		return FALSE;
	}

	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& Param = InstanceParameters(i);
		if (Param.Name == InName && Param.ParamType == PSPT_Actor)
		{
			OutActor = Param.Actor;
			return TRUE;
		}
	}

	return FALSE;
}

/**
 *	Retrieve the Material parameter value for the given name.
 *
 *	@param	InName		Name of the parameter
 *	@param	OutMaterial	The value of the parameter found
 *
 *	@return	TRUE		Parameter was found - OutMaterial is valid
 *			FALSE		Parameter was not found - OutMaterial is invalid
 */
UBOOL UParticleSystemComponent::GetMaterialParameter(const FName InName,class UMaterialInterface*& OutMaterial)
{
	// Always fail if we pass in no name.
	if (InName == NAME_None)
	{
		return FALSE;
	}

	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		FParticleSysParam& Param = InstanceParameters(i);
		if (Param.Name == InName && Param.ParamType == PSPT_Material)
		{
			OutMaterial = Param.Material;
			return TRUE;
		}
	}

	return FALSE;
}

/** clears the specified parameter, returning it to the default value set in the template
 * @param ParameterName name of parameter to remove
 * @param ParameterType type of parameter to remove; if omitted or PSPT_None is specified, all parameters with the given name are removed
 */
void UParticleSystemComponent::ClearParameter(FName ParameterName, BYTE ParameterType)
{
	for (INT i = 0; i < InstanceParameters.Num(); i++)
	{
		if (InstanceParameters(i).Name == ParameterName && (ParameterType == PSPT_None || InstanceParameters(i).ParamType == ParameterType))
		{
			InstanceParameters.Remove(i--);
		}
	}
}

/** 
 *	Auto-populate the instance parameters based on contained modules.
 */
void UParticleSystemComponent::AutoPopulateInstanceProperties()
{
	if (Template)
	{
		for (INT EmitterIndex = 0; EmitterIndex < Template->Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter* Emitter = Template->Emitters(EmitterIndex);
			Emitter->AutoPopulateInstanceProperties(this);
		}
	}
}

void UParticleSystemComponent::FlushSMComponentsArray()
{
	if (GWorld && GWorld->GetWorldInfo())
	{
		AEmitterPool* EmitterPool = GWorld->GetWorldInfo()->MyEmitterPool;
		if (EmitterPool)
		{
			EmitterPool->FreeStaticMeshComponents(this);
		}
	}
	SMComponents.Empty();
	SMMaterialInterfaces.Empty();
}

/** 
 *	Record a spawning event. 
 *
 *	@param	InEventName			The name of the event that fired.
 *	@param	InEmitterTime		The emitter time when the event fired.
 *	@param	InLocation			The location of the particle when the event fired.
 *	@param	InVelocity			The velocity of the particle when the event fired.
 */
void UParticleSystemComponent::ReportEventSpawn(FName& InEventName, FLOAT InEmitterTime, 
	FVector& InLocation, FVector& InVelocity)
{
	FParticleEventSpawnData* SpawnData = new(SpawnEvents)FParticleEventSpawnData;
	SpawnData->Type = EPET_Spawn;
	SpawnData->EventName = InEventName;
	SpawnData->EmitterTime = InEmitterTime;
	SpawnData->Location = InLocation;
	SpawnData->Velocity = InVelocity;
}

/** 
 *	Record a death event.
 *
 *	@param	InEventName			The name of the event that fired.
 *	@param	InEmitterTime		The emitter time when the event fired.
 *	@param	InLocation			The location of the particle when the event fired.
 *	@param	InVelocity			The velocity of the particle when the event fired.
 *	@param	InParticleTime		The relative life of the particle when the event fired.
 */
void UParticleSystemComponent::ReportEventDeath(FName& InEventName, FLOAT InEmitterTime, 
	FVector& InLocation, FVector& InVelocity, FLOAT InParticleTime)
{
	FParticleEventDeathData* DeathData = new(DeathEvents)FParticleEventDeathData;
	DeathData->Type = EPET_Death;
	DeathData->EventName = InEventName;
	DeathData->EmitterTime = InEmitterTime;
	DeathData->Location = InLocation;
	DeathData->Velocity = InVelocity;
	DeathData->ParticleTime = InParticleTime;
}

/** 
 *	Record a collision event.
 *
 *	@param	InEventName		The name of the event that fired.
 *	@param	InEmitterTime	The emitter time when the event fired.
 *	@param	InLocation		The location of the particle when the event fired.
 *	@param	InDirection		The direction of the particle when the event fired.
 *	@param	InVelocity		The velocity of the particle when the event fired.
 *	@param	InParticleTime	The relative life of the particle when the event fired.
 *	@param	InNormal		Normal vector of the collision in coordinate system of the returner. Zero=none.
 *	@param	InTime			Time until hit, if line check.
 *	@param	InItem			Primitive data item which was hit, INDEX_NONE=none.
 *	@param	InBoneName		Name of bone we hit (for skeletal meshes).
 */
void UParticleSystemComponent::ReportEventCollision(FName& InEventName, FLOAT InEmitterTime, 
	FVector& InLocation, FVector& InDirection, FVector& InVelocity, FLOAT InParticleTime, 
	FVector& InNormal, FLOAT InTime, INT InItem, FName& InBoneName)
{
	FParticleEventCollideData* CollideData = new(CollisionEvents)FParticleEventCollideData;
	CollideData->Type = EPET_Collision;
	CollideData->EventName = InEventName;
	CollideData->EmitterTime = InEmitterTime;
	CollideData->Location = InLocation;
	CollideData->Direction = InDirection;
	CollideData->Velocity = InVelocity;
	CollideData->ParticleTime = InParticleTime;
	CollideData->Normal = InNormal;
	CollideData->Time = InTime;
	CollideData->Item = InItem;
	CollideData->BoneName = InBoneName;
}

/** 
 *	Record a kismet event.
 *
 *	@param	InEventName				The name of the event that fired.
 *	@param	InEmitterTime			The emitter time when the event fired.
 *	@param	InLocation				The location of the particle when the event fired.
 *	@param	InVelocity				The velocity of the particle when the event fired.
 *	@param	bInUsePSysCompLocation	If TRUE, use the particle system component location as spawn location.
 *	@param	InNormal				Normal vector of the collision in coordinate system of the returner. Zero=none.
 */
void UParticleSystemComponent::ReportEventKismet(FName& InEventName, FLOAT InEmitterTime, FVector& InLocation, 
	FVector& InDirection, FVector& InVelocity, UBOOL bInUsePSysCompLocation, FVector& InNormal)
{
	FParticleEventKismetData* KismetData = new(KismetEvents)FParticleEventKismetData;
	KismetData->Type = EPET_Kismet;
	KismetData->EventName = InEventName;
	KismetData->EmitterTime = InEmitterTime;
	KismetData->Location = InLocation;
	KismetData->Direction = InDirection;
	KismetData->Velocity = InVelocity;
	KismetData->UsePSysCompLocation = bInUsePSysCompLocation;
	KismetData->Normal = InNormal;
}

void UParticleSystemComponent::KillParticlesForced()
{
	for (INT EmitterIndex=0;EmitterIndex<EmitterInstances.Num();EmitterIndex++)
	{
		if (EmitterInstances(EmitterIndex))
		{
			EmitterInstances(EmitterIndex)->KillParticlesForced();
		}
	}
}

/**
 *	Function for setting the bSkipUpdateDynamicDataDuringTick flag.
 */
void UParticleSystemComponent::SetSkipUpdateDynamicDataDuringTick(UBOOL bInSkipUpdateDynamicDataDuringTick)
{
	bSkipUpdateDynamicDataDuringTick = bInSkipUpdateDynamicDataDuringTick;
}

/**
 *	Function for retrieving the bSkipUpdateDynamicDataDuringTick flag.
 */
UBOOL UParticleSystemComponent::GetSkipUpdateDynamicDataDuringTick()
{
	return bSkipUpdateDynamicDataDuringTick;
}



/**
 * Finds the replay clip of the specified ID number
 *
 * @return Returns the replay clip or NULL if none
 */
UParticleSystemReplay* UParticleSystemComponent::FindReplayClipForIDNumber( const INT InClipIDNumber )
{
	// @todo: If we ever end up with more than a few clips, consider changing this to a hash
	for( INT CurClipIndex = 0; CurClipIndex < ReplayClips.Num(); ++CurClipIndex )
	{
		UParticleSystemReplay* CurReplayClip = ReplayClips( CurClipIndex );
		if( CurReplayClip != NULL )
		{
			if( CurReplayClip->ClipIDNumber == InClipIDNumber )
			{
				// Found it!  We're done.
				return CurReplayClip;
			}
		}
	}

	// Not found
	return NULL;
}

/**
 *	Freeing up dynamic data... 
 *	Due to RenderResources being held by the GameThread when double-buffering,
 *	the system must now track the release of these items...
 */
void UParticleSystemComponent::HandleDynamicDataDeletion(FParticleDynamicData* DynamicData, FDynamicEmitterDataBase* EmitterDynamicData)
{
#if PARTICLES_USE_DOUBLE_BUFFERING
	if (DynamicData)
	{
		checkf(((DynamicDataBuffers[0] && DynamicDataBuffers[0]->DynamicData && (DynamicDataBuffers[0]->DynamicData == DynamicData)) ||
			(DynamicDataBuffers[1] && DynamicDataBuffers[1]->DynamicData && (DynamicDataBuffers[1]->DynamicData == DynamicData))),
			TEXT("HandleDynamicDataDeletion> PSysComp: 0x%08x - Didn't own 0x%08x"), (DWORD)this, (DWORD)DynamicData);
		// Create the tracker if it hasn't been yet...
		if (DeferredReleaseItems == NULL)
		{
			DeferredReleaseItems = new FParticleDeferredReleases();
			check(DeferredReleaseItems);
		}

		// Add the item to the list...
		FParticleDeferredReleaseResources* NewDeferred = new(DeferredReleaseItems->DeferredReleases)FParticleDeferredReleaseResources;
		check(NewDeferred);

		NewDeferred->DynamicData = DynamicData;
		NewDeferred->EmitterDynamicData = EmitterDynamicData;
		if (DynamicData)
		{
			DynamicData->CleanupDataForDeferredDeletion();
		}
		if (EmitterDynamicData)
		{
			EmitterDynamicData->ReleaseResource();
		}
		NewDeferred->ReleaseFence = new FRenderCommandFence();
		check(NewDeferred->ReleaseFence);
		NewDeferred->ReleaseFence->BeginFence();
	}
#endif	//#if PARTICLES_USE_DOUBLE_BUFFERING
}

/** UParticleSystemReplay serialization */
void UParticleSystemReplay::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	// Serialize clip ID number
	Ar << ClipIDNumber;

	// Serialize our native members
	Ar << Frames;
}



/** FParticleSystemReplayFrame serialization operator */
FArchive& operator<<( FArchive& Ar, FParticleSystemReplayFrame& Obj )
{
	if( Ar.IsLoading() )
	{
		// Zero out the struct if we're loading from disk since we won't be cleared by default
		appMemzero( &Obj, sizeof( FParticleEmitterReplayFrame ) );
	}

	// Serialize emitter frames
	Ar << Obj.Emitters;

	return Ar;
}



/** FParticleEmitterReplayFrame serialization operator */
FArchive& operator<<( FArchive& Ar, FParticleEmitterReplayFrame& Obj )
{
	if( Ar.IsLoading() )
	{
		// Zero out the struct if we're loading from disk since we won't be cleared by default
		appMemzero( &Obj, sizeof( FParticleEmitterReplayFrame ) );
	}

	// Emitter type
	Ar << Obj.EmitterType;

	// Original emitter index
	Ar << Obj.OriginalEmitterIndex;

	if( Ar.IsLoading() )
	{
		switch( Obj.EmitterType )
		{
			case DET_Sprite:
				Obj.FrameState = new FDynamicSpriteEmitterReplayData();
				break;

			case DET_SubUV:
				Obj.FrameState = new FDynamicSubUVEmitterReplayData();
				break;

			case DET_Mesh:
				Obj.FrameState = new FDynamicMeshEmitterReplayData();
				break;

			case DET_Beam2:
				Obj.FrameState = new FDynamicBeam2EmitterReplayData();
				break;

			case DET_Trail2:
				Obj.FrameState = new FDynamicTrail2EmitterReplayData();
				break;

			default:
				{
					// @todo: Support other particle types
					Obj.FrameState = NULL;
				}
				break;
		}
	}

	if( Obj.FrameState != NULL )
	{
		// Serialize this emitter frame state
		Obj.FrameState->Serialize( Ar );
	}

	return Ar;
}
