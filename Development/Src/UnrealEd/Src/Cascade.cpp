/*=============================================================================
	Cascade.cpp: 'Cascade' particle editor
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "Cascade.h"
#include "CurveEd.h"
#include "EngineMaterialClasses.h"
#include "Properties.h"
#include "GenericBrowser.h"

IMPLEMENT_CLASS(UCascadeOptions);
IMPLEMENT_CLASS(UCascadePreviewComponent);

/*-----------------------------------------------------------------------------
	UCascadeParticleSystemComponent
-----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(UCascadeParticleSystemComponent);

UBOOL UCascadeParticleSystemComponent::SingleLineCheck(FCheckResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, DWORD TraceFlags, const FVector& Extent)
{
	if (CascadePreviewViewportPtr && CascadePreviewViewportPtr->FloorComponent && (CascadePreviewViewportPtr->FloorComponent->HiddenEditor == FALSE))
	{
		Hit = FCheckResult(1.f);

		const UBOOL bHit = !CascadePreviewViewportPtr->FloorComponent->LineCheck(Hit, End, Start, Extent, TraceFlags);
		if (bHit)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/*-----------------------------------------------------------------------------
	FCascadeNotifyHook
-----------------------------------------------------------------------------*/
void FCascadeNotifyHook::NotifyDestroy(void* Src)
{
	if (WindowOfInterest == Src)
	{
		if (Cascade->PreviewVC && Cascade->PreviewVC->FloorComponent)
		{
			// Save the information in the particle system
			Cascade->PartSys->FloorPosition = Cascade->PreviewVC->FloorComponent->Translation;
			Cascade->PartSys->FloorRotation = Cascade->PreviewVC->FloorComponent->Rotation;
			Cascade->PartSys->FloorScale = Cascade->PreviewVC->FloorComponent->Scale;
			Cascade->PartSys->FloorScale3D = Cascade->PreviewVC->FloorComponent->Scale3D;

			if (Cascade->PreviewVC->FloorComponent->StaticMesh)
			{
				Cascade->PartSys->FloorMesh = Cascade->PreviewVC->FloorComponent->StaticMesh->GetPathName();
			}
			else
			{
				warnf(TEXT("Unable to locate Cascade floor mesh outer..."));
				Cascade->PartSys->FloorMesh = TEXT("");
			}
			Cascade->PartSys->MarkPackageDirty();
		}
	}
}

/*-----------------------------------------------------------------------------
	WxCascade
-----------------------------------------------------------------------------*/

UBOOL				WxCascade::bParticleModuleClassesInitialized = FALSE;
TArray<UClass*>		WxCascade::ParticleModuleClasses;
TArray<UClass*>		WxCascade::ParticleModuleBaseClasses;

// On init, find all particle module classes. Will use later on to generate menus.
void WxCascade::InitParticleModuleClasses()
{
	if (bParticleModuleClassesInitialized)
		return;

	for(TObjectIterator<UClass> It; It; ++It)
	{
		// Find all ParticleModule classes (ignoring abstract or ParticleTrailModule classes
		if (It->IsChildOf(UParticleModule::StaticClass()))
		{
			if (!(It->ClassFlags & CLASS_Abstract) &&
				(*It != UParticleModuleTypeDataTrail::StaticClass()) &&
				(*It != UParticleModuleTypeDataBeam::StaticClass()) &&
				(*It != UParticleModuleLocationPrimitiveBase::StaticClass()) &&
				// Spawn module is NOT allowed to be in the list...
				(*It != UParticleModuleSpawn::StaticClass()) &&
				// Required module either!
				(*It != UParticleModuleRequired::StaticClass())
				)
			{
				ParticleModuleClasses.AddItem(*It);
			}
			else
			{
				if (!(*It == UParticleModuleUberBase::StaticClass()) &&
					(*It != UParticleModuleTypeDataTrail::StaticClass()) &&
					(*It != UParticleModuleTypeDataBeam::StaticClass()) &&
					(*It != UParticleModuleLocationPrimitiveBase::StaticClass()) &&
					(*It != UParticleModuleSpawn::StaticClass()))
				{
					ParticleModuleBaseClasses.AddItem(*It);
				}
			}
		}
	}

	bParticleModuleClassesInitialized = TRUE;
}

bool WxCascade::DuplicateEmitter(UParticleEmitter* SourceEmitter, UParticleSystem* DestSystem, UBOOL bShare)
{
	UObject* SourceOuter = SourceEmitter->GetOuter();
	if (SourceOuter != DestSystem)
	{
		if (bShare == TRUE)
		{
			warnf(TEXT("Can't share modules across particle systems!"));
			bShare = FALSE;
		}
	}

	INT InsertionIndex = -1;
	if (SourceOuter == DestSystem)
	{
		UParticleSystem* SourcePSys = Cast<UParticleSystem>(SourceOuter);
		if (SourcePSys)
		{
			// Find the source emitter in the SourcePSys emitter array
			for (INT CheckSourceIndex = 0; CheckSourceIndex < SourcePSys->Emitters.Num(); CheckSourceIndex++)
			{
				if (SourcePSys->Emitters(CheckSourceIndex) == SourceEmitter)
				{
					InsertionIndex = CheckSourceIndex + 1;
					break;
				}
			}
		}
	}

	// Find desired class of new module.
    UClass* NewEmitClass = SourceEmitter->GetClass();
	if (NewEmitClass == UParticleSpriteEmitter::StaticClass())
	{
		// Construct it
		UParticleEmitter* NewEmitter = ConstructObject<UParticleEmitter>(NewEmitClass, DestSystem, NAME_None, RF_Transactional);

		check(NewEmitter);

		FString	NewName = SourceEmitter->GetEmitterName().ToString();

		NewEmitter->SetEmitterName(FName(*NewName));

		//	'Private' data - not required by the editor
		UObject*			DupObject;
		UParticleLODLevel*	SourceLODLevel;
		UParticleLODLevel*	NewLODLevel;
		UParticleLODLevel*	PrevSourceLODLevel = NULL;
		UParticleLODLevel*	PrevLODLevel = NULL;

		NewEmitter->LODLevels.InsertZeroed(0, SourceEmitter->LODLevels.Num());
		for (INT LODIndex = 0; LODIndex < SourceEmitter->LODLevels.Num(); LODIndex++)
		{
			SourceLODLevel	= SourceEmitter->LODLevels(LODIndex);
			NewLODLevel		= ConstructObject<UParticleLODLevel>(UParticleLODLevel::StaticClass(), NewEmitter, NAME_None, RF_Transactional);
			check(NewLODLevel);

			NewLODLevel->Level					= SourceLODLevel->Level;
			NewLODLevel->bEnabled				= SourceLODLevel->bEnabled;

			// The RequiredModule
			if (bShare)
			{
				NewLODLevel->RequiredModule = SourceLODLevel->RequiredModule;
			}
			else
			{
				if ((LODIndex > 0) && (PrevSourceLODLevel->RequiredModule == SourceLODLevel->RequiredModule))
				{
					PrevLODLevel->RequiredModule->LODValidity |= (1 << LODIndex);
					NewLODLevel->RequiredModule = PrevLODLevel->RequiredModule;
				}
				else
				{
					DupObject = GEditor->StaticDuplicateObject(SourceLODLevel->RequiredModule, SourceLODLevel->RequiredModule, DestSystem, TEXT("None"));
					check(DupObject);
					NewLODLevel->RequiredModule						= Cast<UParticleModuleRequired>(DupObject);
					NewLODLevel->RequiredModule->ModuleEditorColor	= FColor::MakeRandomColor();
					NewLODLevel->RequiredModule->LODValidity		= (1 << LODIndex);
				}
			}

			// The SpawnModule
			if (bShare)
			{
				NewLODLevel->SpawnModule = SourceLODLevel->SpawnModule;
			}
			else
			{
				if ((LODIndex > 0) && (PrevSourceLODLevel->SpawnModule == SourceLODLevel->SpawnModule))
				{
					PrevLODLevel->SpawnModule->LODValidity |= (1 << LODIndex);
					NewLODLevel->SpawnModule = PrevLODLevel->SpawnModule;
				}
				else
				{
					DupObject = GEditor->StaticDuplicateObject(SourceLODLevel->SpawnModule, SourceLODLevel->SpawnModule, DestSystem, TEXT("None"));
					check(DupObject);
					NewLODLevel->SpawnModule					= Cast<UParticleModuleSpawn>(DupObject);
					NewLODLevel->SpawnModule->ModuleEditorColor	= FColor::MakeRandomColor();
					NewLODLevel->SpawnModule->LODValidity		= (1 << LODIndex);
				}
			}

			// Copy each module
			NewLODLevel->Modules.InsertZeroed(0, SourceLODLevel->Modules.Num());
			for (INT ModuleIndex = 0; ModuleIndex < SourceLODLevel->Modules.Num(); ModuleIndex++)
			{
				UParticleModule* SourceModule = SourceLODLevel->Modules(ModuleIndex);
				if (bShare)
				{
					NewLODLevel->Modules(ModuleIndex) = SourceModule;
				}
				else
				{
					if ((LODIndex > 0) && (PrevSourceLODLevel->Modules(ModuleIndex) == SourceLODLevel->Modules(ModuleIndex)))
					{
						PrevLODLevel->Modules(ModuleIndex)->LODValidity |= (1 << LODIndex);
						NewLODLevel->Modules(ModuleIndex) = PrevLODLevel->Modules(ModuleIndex);
					}
					else
					{
						DupObject = GEditor->StaticDuplicateObject(SourceModule, SourceModule, DestSystem, TEXT("None"));
						if (DupObject)
						{
							UParticleModule* Module				= Cast<UParticleModule>(DupObject);
							Module->ModuleEditorColor			= FColor::MakeRandomColor();
							NewLODLevel->Modules(ModuleIndex)	= Module;
						}
					}
				}
			}

			// TypeData module as well...
			if (SourceLODLevel->TypeDataModule)
			{
				if (bShare)
				{
					NewLODLevel->TypeDataModule = SourceLODLevel->TypeDataModule;
				}
				else
				{
					if ((LODIndex > 0) && (PrevSourceLODLevel->TypeDataModule == SourceLODLevel->TypeDataModule))
					{
						PrevLODLevel->TypeDataModule->LODValidity |= (1 << LODIndex);
						NewLODLevel->TypeDataModule = PrevLODLevel->TypeDataModule;
					}
					else
					{
						DupObject = GEditor->StaticDuplicateObject(SourceLODLevel->TypeDataModule, SourceLODLevel->TypeDataModule, DestSystem, TEXT("None"));
						if (DupObject)
						{
							UParticleModule* Module		= Cast<UParticleModule>(DupObject);
							Module->ModuleEditorColor	= FColor::MakeRandomColor();
							NewLODLevel->TypeDataModule	= Module;
						}
					}
				}
			}
			NewLODLevel->ConvertedModules		= TRUE;
			NewLODLevel->PeakActiveParticles	= SourceLODLevel->PeakActiveParticles;

			NewEmitter->LODLevels(LODIndex)		= NewLODLevel;

			PrevLODLevel = NewLODLevel;
			PrevSourceLODLevel = SourceLODLevel;
		}

		//@todo. Compare against the destination system, and insert appropriate LOD levels where necessary
		// Generate all the levels that are present in other emitters...
		// NOTE: Big assumptions - the highest and lowest are 0,100 respectively and they MUST exist.
		if (DestSystem->Emitters.Num() > 0)
		{
			UParticleEmitter* DestEmitter = DestSystem->Emitters(0);
			INT DestLODCount = DestEmitter->LODLevels.Num();
			INT NewLODCount = NewEmitter->LODLevels.Num();
			if (DestLODCount != NewLODCount)
			{
				debugf(TEXT("Generating existing LOD levels..."));

				if (DestLODCount < NewLODCount)
				{
					for (INT DestEmitIndex = 0; DestEmitIndex < DestSystem->Emitters.Num(); DestEmitIndex++)
					{
						UParticleEmitter* DestEmitter = DestSystem->Emitters(DestEmitIndex);
						for (INT InsertIndex = DestLODCount; InsertIndex < NewLODCount; InsertIndex++)
						{
							DestEmitter->CreateLODLevel(InsertIndex);
						}
						DestEmitter->UpdateModuleLists();
					}
				}
				else
				{
					for (INT InsertIndex = NewLODCount; InsertIndex < DestLODCount; InsertIndex++)
					{
						NewEmitter->CreateLODLevel(InsertIndex);
					}
				}
			}
		}

        NewEmitter->UpdateModuleLists();

		// Add to selected emitter
		if ((InsertionIndex >= 0) && (InsertionIndex < DestSystem->Emitters.Num()))
		{
			DestSystem->Emitters.InsertItem(NewEmitter, InsertionIndex);
		}
		else
		{
	        DestSystem->Emitters.AddItem(NewEmitter);
		}
	}
	else
	{
		appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Prompt_4"), *NewEmitClass->GetDesc()));
		return FALSE;
	}

	return TRUE;
}

// Undo/Redo support
bool WxCascade::BeginTransaction(const TCHAR* pcTransaction)
{
	if (TransactionInProgress())
	{
		FString kError(*LocalizeUnrealEd("Error_FailedToBeginTransaction"));
		kError += kTransactionName;
		checkf(0, TEXT("%s"), *kError);
		return FALSE;
	}

	GEditor->BeginTransaction(pcTransaction);
	kTransactionName = FString(pcTransaction);
	bTransactionInProgress = TRUE;

	return TRUE;
}

bool WxCascade::EndTransaction(const TCHAR* pcTransaction)
{
	if (!TransactionInProgress())
	{
		FString kError(*LocalizeUnrealEd("Error_FailedToEndTransaction"));
		kError += kTransactionName;
		checkf(0, TEXT("%s"), *kError);
		return FALSE;
	}

	if (appStrcmp(*kTransactionName, pcTransaction) != 0)
	{
		debugf(TEXT("Cascade -   EndTransaction = %s --- Curr = %s"), 
			pcTransaction, *kTransactionName);
		return FALSE;
	}

	GEditor->EndTransaction();

	kTransactionName = TEXT("");
	bTransactionInProgress = FALSE;

	return TRUE;
}

bool WxCascade::TransactionInProgress()
{
	return bTransactionInProgress;
}

void WxCascade::ModifySelectedObjects()
{
	if (SelectedEmitter)
	{
		ModifyEmitter(SelectedEmitter);
	}
	if (SelectedModule)
	{
		SelectedModule->Modify();
	}
}

void WxCascade::ModifyParticleSystem()
{
	PartSys->Modify();
	PartSysComp->Modify();
}

void WxCascade::ModifyEmitter(UParticleEmitter* Emitter)
{
	if (Emitter)
	{
		Emitter->Modify();
		for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
		{
			UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
			if (LODLevel)
			{
				LODLevel->Modify();
			}
		}
	}
}

void WxCascade::CascadeUndo()
{
	GEditor->UndoTransaction();

	CascadeTouch();
	wxCommandEvent DummyEvent;
	OnResetInLevel(DummyEvent);
}

void WxCascade::CascadeRedo()
{
	GEditor->RedoTransaction();

	CascadeTouch();
	wxCommandEvent DummyEvent;
	OnResetInLevel(DummyEvent);
}

void WxCascade::CascadeTouch()
{
	// Touch the module lists in each emitter.
	for (INT ii = 0; ii < PartSys->Emitters.Num(); ii++)
	{
		UParticleEmitter* pkEmitter = PartSys->Emitters(ii);
		pkEmitter->UpdateModuleLists();
	}
	UpdateLODLevelControls();
	PartSysComp->ResetParticles();
	PartSysComp->InitializeSystem();
	// 'Refresh' the viewport
	EmitterEdVC->Viewport->Invalidate();
	CurveEd->CurveChanged();
}

void WxCascade::UpdateLODLevelControls()
{
	INT CurrentLODLevel = GetCurrentlySelectedLODLevelIndex();
	SetLODValue(CurrentLODLevel);
}

// PostProces
/**
 *	Update the post process chain according to the show options
 */
void WxCascade::UpdatePostProcessChain()
{
	if (DefaultPostProcess && PreviewVC)
	{
		UPostProcessEffect* BloomEffect = NULL;
		UPostProcessEffect* DOFEffect = NULL;
		UPostProcessEffect* MotionBlurEffect = NULL;
		UPostProcessEffect* PPVolumeEffect = NULL;

		for (INT EffectIndex = 0; EffectIndex < DefaultPostProcess->Effects.Num(); EffectIndex++)
		{
			UPostProcessEffect* Effect = DefaultPostProcess->Effects(EffectIndex);
			if (Effect)
			{
				if (Effect->EffectName.ToString() == FString(TEXT("CascadeDOFAndBloom")))
				{
					BloomEffect = Effect;
					DOFEffect = Effect;
				}
				else
				if (Effect->EffectName.ToString() == FString(TEXT("CascadeMotionBlur")))
				{
					MotionBlurEffect = Effect;
				}
				else
				if (Effect->EffectName.ToString() == FString(TEXT("CascadePPVolumeMaterial")))
				{
					PPVolumeEffect = Effect;
				}
			}
		}

		if (BloomEffect)
		{
			if (PreviewVC->ShowPPFlags & CASC_SHOW_BLOOM)
			{
				BloomEffect->bShowInEditor = TRUE;
				BloomEffect->bShowInGame = TRUE;
			}
			else
			{
				BloomEffect->bShowInEditor = FALSE;
				BloomEffect->bShowInGame = FALSE;
			}
		}
		if (DOFEffect)
		{
			if (PreviewVC->ShowPPFlags & CASC_SHOW_DOF)
			{
				DOFEffect->bShowInEditor = TRUE;
				DOFEffect->bShowInGame = TRUE;
			}
			else
			{
				DOFEffect->bShowInEditor = FALSE;
				DOFEffect->bShowInGame = FALSE;
			}
		}
		if (MotionBlurEffect)
		{
			if (PreviewVC->ShowPPFlags & CASC_SHOW_MOTIONBLUR)
			{
				MotionBlurEffect->bShowInEditor = TRUE;
				MotionBlurEffect->bShowInGame = TRUE;
			}
			else
			{
				MotionBlurEffect->bShowInEditor = FALSE;
				MotionBlurEffect->bShowInGame = FALSE;
			}
		}
		if (PPVolumeEffect)
		{
			if (PreviewVC->ShowPPFlags & CASC_SHOW_PPVOLUME)
			{
				PPVolumeEffect->bShowInEditor = TRUE;
				PPVolumeEffect->bShowInGame = TRUE;
			}
			else
			{
				PPVolumeEffect->bShowInEditor = FALSE;
				PPVolumeEffect->bShowInGame = FALSE;
			}
		}
	}
	else
	{
		warnf(TEXT("CASCADE::UpdatePostProcessChain> Nno post process chain."));
	}
}

/**
 *	Return the currently selected LOD level
 *
 *	@return	INT		The currently selected LOD level...
 */
INT WxCascade::GetCurrentlySelectedLODLevelIndex()
{
	if (ToolBar)
	{
		FString CurrLODText = ToolBar->LODCurrent->GetValue().c_str();
		INT SetLODLevelIndex = appAtoi(*CurrLODText) - 1;
		if (SetLODLevelIndex < 0)
		{
			SetLODLevelIndex = 0;
		}
		else
		{
			if (PartSys && (PartSys->Emitters.Num() > 0))
			{
				UParticleEmitter* Emitter = PartSys->Emitters(0);
				if (Emitter)
				{
					if (SetLODLevelIndex >= Emitter->LODLevels.Num())
					{
						SetLODLevelIndex = Emitter->LODLevels.Num() - 1;
					}
				}
			}
			else
			{
				SetLODLevelIndex = 0;
			}
		}

		return SetLODLevelIndex;
	}

	return -1;
}

/**
 *	Return the currently selected LOD level
 *
 *	@return	UParticleLODLevel*	The currently selected LOD level...
 */
UParticleLODLevel* WxCascade::GetCurrentlySelectedLODLevel()
{
	INT CurrentLODLevel = GetCurrentlySelectedLODLevelIndex();
	if ((CurrentLODLevel >= 0) && SelectedEmitter)
	{
		return SelectedEmitter->GetLODLevel(CurrentLODLevel);
	}

	return NULL;
}

/**
 *	Return the currently selected LOD level
 *
 *	@param	InEmitter			The emitter to retrieve it from.
 *	@return	UParticleLODLevel*	The currently selected LOD level.
 */
UParticleLODLevel* WxCascade::GetCurrentlySelectedLODLevel(UParticleEmitter* InEmitter)
{
	if (InEmitter)
	{
		UParticleEmitter* SaveSelectedEmitter = SelectedEmitter;
		SelectedEmitter = InEmitter;
		UParticleLODLevel* ReturnLODLevel = GetCurrentlySelectedLODLevel();
		SelectedEmitter = SaveSelectedEmitter;
		return ReturnLODLevel;
	}

	return NULL;
}

/**
 *	Is the module of the given name suitable for the right-click module menu?
 */
UBOOL WxCascade::IsModuleSuitableForModuleMenu(FString& InModuleName)
{
	INT RejectIndex;
	check(EditorOptions);
	return (EditorOptions->ModuleMenu_ModuleRejections.FindItem(InModuleName, RejectIndex) == FALSE);
}

/**
 *	Is the base module of the given name suitable for the right-click module menu
 *	given the currently selected emitter TypeData?
 */
UBOOL WxCascade::IsBaseModuleTypeDataPairSuitableForModuleMenu(FString& InModuleName)
{
	INT RejectIndex;
	check(EditorOptions);

	FString TDName(TEXT("None"));
	if (SelectedEmitter)
	{
		UParticleLODLevel* LODLevel = GetCurrentlySelectedLODLevel();
		if (LODLevel && LODLevel->TypeDataModule)
		{
			TDName = LODLevel->TypeDataModule->GetClass()->GetName();
		}
	}

	FModuleMenuMapper* Mapper = NULL;
	for (INT MapIndex = 0; MapIndex < EditorOptions->ModuleMenu_TypeDataToBaseModuleRejections.Num(); MapIndex++)
	{
		if (EditorOptions->ModuleMenu_TypeDataToBaseModuleRejections(MapIndex).ObjName == TDName)
		{
			Mapper = &(EditorOptions->ModuleMenu_TypeDataToBaseModuleRejections(MapIndex));
			break;
		}
	}

	if (Mapper)
	{
		if (Mapper->InvalidObjNames.FindItem(InModuleName, RejectIndex) == TRUE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/**
 *	Is the base module of the given name suitable for the right-click module menu
 *	given the currently selected emitter TypeData?
 */
UBOOL WxCascade::IsModuleTypeDataPairSuitableForModuleMenu(FString& InModuleName)
{
	INT RejectIndex;
	check(EditorOptions);

	FString TDName(TEXT("None"));
	if (SelectedEmitter)
	{
		UParticleLODLevel* LODLevel = GetCurrentlySelectedLODLevel();
		if (LODLevel && LODLevel->TypeDataModule)
		{
			TDName = LODLevel->TypeDataModule->GetClass()->GetName();
		}
	}

	FModuleMenuMapper* Mapper = NULL;
	for (INT MapIndex = 0; MapIndex < EditorOptions->ModuleMenu_TypeDataToSpecificModuleRejections.Num(); MapIndex++)
	{
		if (EditorOptions->ModuleMenu_TypeDataToSpecificModuleRejections(MapIndex).ObjName == TDName)
		{
			Mapper = &(EditorOptions->ModuleMenu_TypeDataToSpecificModuleRejections(MapIndex));
			break;
		}
	}

	if (Mapper)
	{
		if (Mapper->InvalidObjNames.FindItem(InModuleName, RejectIndex) == TRUE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/**
 */
void WxCascade::SetLODValue(INT LODSetting)
{
	if (ToolBar)
	{
		FString	ValueStr = FString::Printf(TEXT("%d"), LODSetting + 1);
		ToolBar->LODCurrent->SetValue(*ValueStr);
		INT LODCount = PartSys ? (PartSys->Emitters.Num() > 0) ? PartSys->Emitters(0)->LODLevels.Num() : -1 : -2;
		ValueStr = FString::Printf(TEXT("Total = %d"), LODCount);
		ToolBar->LODTotal->SetValue(*ValueStr);
	}

	if (LODSetting >= 0)
	{
		if (PartSys)
		{
			PartSys->EditorLODSetting	= LODSetting;
		}
		if (PartSysComp)
		{
			PartSysComp->EditorLODLevel = LODSetting;
			PartSysComp->SetLODLevel(LODSetting);
		}
	}
}

BEGIN_EVENT_TABLE(WxCascade, WxTrackableFrame)
	EVT_SIZE(WxCascade::OnSize)
	EVT_MENU(IDM_CASCADE_RENAME_EMITTER, WxCascade::OnRenameEmitter)
	EVT_MENU_RANGE(	IDM_CASCADE_NEW_EMITTER_START, IDM_CASCADE_NEW_EMITTER_END, WxCascade::OnNewEmitter )
	EVT_MENU(IDM_CASCADE_SELECT_PARTICLESYSTEM, WxCascade::OnSelectParticleSystem)
	EVT_MENU(IDM_CASCADE_PSYS_NEW_EMITTER_BEFORE, WxCascade::OnNewEmitterBefore)
	EVT_MENU(IDM_CASCADE_PSYS_NEW_EMITTER_AFTER, WxCascade::OnNewEmitterAfter) 
	EVT_MENU_RANGE( IDM_CASCADE_NEW_MODULE_START, IDM_CASCADE_NEW_MODULE_END, WxCascade::OnNewModule )
	EVT_MENU(IDM_CASCADE_ADD_SELECTED_MODULE, WxCascade::OnAddSelectedModule)
	EVT_MENU(IDM_CASCADE_COPY_MODULE, WxCascade::OnCopyModule)
	EVT_MENU(IDM_CASCADE_PASTE_MODULE, WxCascade::OnPasteModule)
	EVT_MENU( IDM_CASCADE_DELETE_MODULE, WxCascade::OnDeleteModule )
	EVT_MENU( IDM_CASCADE_ENABLE_MODULE, WxCascade::OnEnableModule )
	EVT_MENU( IDM_CASCADE_RESET_MODULE, WxCascade::OnResetModule )
	EVT_MENU( IDM_CASCADE_REFRESH_MODULE, WxCascade::OnRefreshModule )
	EVT_MENU( IDM_CASCADE_MODULE_SYNCMATERIAL,  WxCascade::OnModuleSyncMaterial )
	EVT_MENU( IDM_CASCADE_MODULE_USEMATERIAL, WxCascade::OnModuleUseMaterial )
	EVT_MENU( IDM_CASCADE_MODULE_DUPHIGHEST, WxCascade::OnModuleDupHighest )
	EVT_MENU( IDM_CASCADE_MODULE_SHAREHIGH, WxCascade::OnModuleShareHigher )
	EVT_MENU( IDM_CASCADE_MODULE_DUPHIGH, WxCascade::OnModuleDupHigher )
	EVT_MENU(IDM_CASCADE_DUPLICATE_EMITTER, WxCascade::OnDuplicateEmitter)
	EVT_MENU(IDM_CASCADE_DUPLICATE_SHARE_EMITTER, WxCascade::OnDuplicateEmitter)
	EVT_MENU( IDM_CASCADE_DELETE_EMITTER, WxCascade::OnDeleteEmitter )
	EVT_MENU(IDM_CASCADE_EXPORT_EMITTER, WxCascade::OnExportEmitter)
	EVT_MENU(IDM_CASCADE_CONVERT_RAIN_EMITTER, WxCascade::OnConvertRainEmitter)
	EVT_MENU_RANGE( IDM_CASCADE_SIM_PAUSE, IDM_CASCADE_SIM_NORMAL, WxCascade::OnMenuSimSpeed )
	EVT_MENU( IDM_CASCADE_SAVECAM, WxCascade::OnSaveCam )
#if defined(_CASCADE_ENABLE_MODULE_DUMP_)
	EVT_MENU( IDM_CASCADE_VIEW_DUMP, WxCascade::OnViewModuleDump)
#endif	//#if defined(_CASCADE_ENABLE_MODULE_DUMP_)
	EVT_MENU( IDM_CASCADE_RESETSYSTEM, WxCascade::OnResetSystem )
	EVT_MENU( IDM_CASCADE_RESETINLEVEL, WxCascade::OnResetInLevel )
	EVT_TOOL( IDM_CASCADE_ORBITMODE, WxCascade::OnOrbitMode )
	EVT_TOOL(IDM_CASCADE_WIREFRAME, WxCascade::OnWireframe)
	EVT_TOOL(IDM_CASCADE_BOUNDS, WxCascade::OnBounds)
	EVT_TOOL_RCLICKED(IDM_CASCADE_BOUNDS, WxCascade::OnBoundsRightClick)
	EVT_TOOL(IDM_CASCADE_POSTPROCESS, WxCascade::OnPostProcess)
	EVT_TOOL(IDM_CASCADE_TOGGLEGRID, WxCascade::OnToggleGrid)
	EVT_TOOL(IDM_CASCADE_PLAY, WxCascade::OnPlay)
	EVT_TOOL(IDM_CASCADE_PAUSE, WxCascade::OnPause)
	EVT_TOOL(IDM_CASCADE_SPEED_100,	WxCascade::OnSpeed)
	EVT_TOOL(IDM_CASCADE_SPEED_50, WxCascade::OnSpeed)
	EVT_TOOL(IDM_CASCADE_SPEED_25, WxCascade::OnSpeed)
	EVT_TOOL(IDM_CASCADE_SPEED_10, WxCascade::OnSpeed)
	EVT_TOOL(IDM_CASCADE_SPEED_1, WxCascade::OnSpeed)
	EVT_TOOL(IDM_CASCADE_LOOPSYSTEM, WxCascade::OnLoopSystem)
	EVT_TOOL(IDM_CASCADE_REALTIME, WxCascade::OnRealtime)
	EVT_TOOL(IDM_CASCADE_BACKGROUND_COLOR, WxCascade::OnBackgroundColor)
	EVT_TOOL(IDM_CASCADE_TOGGLE_WIRE_SPHERE, WxCascade::OnToggleWireSphere)
	EVT_TOOL(IDM_CASCADE_UNDO, WxCascade::OnUndo)
	EVT_TOOL(IDM_CASCADE_REDO, WxCascade::OnRedo)
	EVT_TOOL(IDM_CASCADE_PERFORMANCE_CHECK, WxCascade::OnPerformanceCheck)
	EVT_TOOL(IDM_CASCADE_LOD_LOW, WxCascade::OnLODLow)
	EVT_TOOL(IDM_CASCADE_LOD_LOWER, WxCascade::OnLODLower)
	EVT_TOOL(IDM_CASCADE_LOD_ADDBEFORE, WxCascade::OnLODAddBefore)
	EVT_TOOL(IDM_CASCADE_LOD_ADDAFTER, WxCascade::OnLODAddAfter)
	EVT_TOOL(IDM_CASCADE_LOD_HIGHER, WxCascade::OnLODHigher)
	EVT_TOOL(IDM_CASCADE_LOD_HIGH, WxCascade::OnLODHigh)
	EVT_TOOL(IDM_CASCADE_LOD_DELETE, WxCascade::OnLODDelete)
	EVT_TOOL(IDM_CASCADE_LOD_REGEN, WxCascade::OnRegenerateLowestLOD)
	EVT_TOOL(IDM_CASCADE_LOD_REGENDUP, WxCascade::OnRegenerateLowestLODDuplicateHighest)
	EVT_MENU( IDM_CASCADE_VIEW_AXES, WxCascade::OnViewAxes )
	EVT_MENU(IDM_CASCADE_VIEW_COUNTS, WxCascade::OnViewCounts)
	EVT_MENU(IDM_CASCADE_VIEW_TIMES, WxCascade::OnViewTimes)
	EVT_MENU(IDM_CASCADE_VIEW_EVENTS, WxCascade::OnViewEvents)
	EVT_MENU(IDM_CASCADE_VIEW_DISTANCE, WxCascade::OnViewDistance)
	EVT_MENU(IDM_CASCADE_VIEW_GEOMETRY, WxCascade::OnViewGeometry)
	EVT_MENU(IDM_CASCADE_VIEW_GEOMETRY_PROPERTIES, WxCascade::OnViewGeometryProperties)
	EVT_MENU(IDM_CASCADE_RESET_PEAK_COUNTS, WxCascade::OnResetPeakCounts)
	EVT_MENU(IDM_CASCADE_CONVERT_TO_UBER, WxCascade::OnUberConvert)
	EVT_MENU(IDM_CASCADE_REGENERATE_LOWESTLOD, WxCascade::OnRegenerateLowestLOD)
	EVT_MENU(IDM_CASCADE_SAVE_PACKAGE, WxCascade::OnSavePackage)
	EVT_MENU(IDM_CASCADE_SIM_RESTARTONFINISH, WxCascade::OnLoopSimulation )
	EVT_MENU(IDM_CASC_SHOWPP_BLOOM, WxCascade::OnShowPPBloom)
	EVT_MENU(IDM_CASC_SHOWPP_DOF, WxCascade::OnShowPPDOF)
	EVT_MENU(IDM_CASC_SHOWPP_MOTIONBLUR, WxCascade::OnShowPPMotionBlur)
	EVT_MENU(IDM_CASC_SHOWPP_PPVOLUME, WxCascade::OnShowPPVolumeMaterial)
END_EVENT_TABLE()


#define CASCADE_NUM_SASHES		4

WxCascade::WxCascade(wxWindow* InParent, wxWindowID InID, UParticleSystem* InPartSys) : 
	WxTrackableFrame(InParent, InID, TEXT(""), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT | wxFRAME_NO_TASKBAR, 
		InPartSys ? *(InPartSys->GetPathName()) : TEXT("EMPTY")),
	FDockingParent(this),
	MenuBar(NULL),
	ToolBar(NULL),
	PreviewVC(NULL)
{
	check(InPartSys);
	InPartSys->EditorLODSetting	= 0;

	DefaultPostProcessName = TEXT("");
    DefaultPostProcess = NULL;

	EditorOptions = ConstructObject<UCascadeOptions>(UCascadeOptions::StaticClass());
	check(EditorOptions);

	// Load the desired window position from .ini file
	FWindowUtil::LoadPosSize(TEXT("CascadeEditor"), this, 256, 256, 1024, 768);
	
	// Make sure we have a list of available particle modules
	WxCascade::InitParticleModuleClasses();

	// Set up pointers to interp objects
	PartSys = InPartSys;

	// Set up for undo/redo!
	PartSys->SetFlags(RF_Transactional);

	for (INT ii = 0; ii < PartSys->Emitters.Num(); ii++)
	{
		UParticleEmitter* Emitter = PartSys->Emitters(ii);
		if (Emitter)
		{
			Emitter->SetFlags(RF_Transactional);
			for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
			{
				UParticleLODLevel* LODLevel = Emitter->GetLODLevel(LODIndex);
				if (LODLevel)
				{
					LODLevel->SetFlags(RF_Transactional);
					check(LODLevel->RequiredModule);
					LODLevel->RequiredModule->SetFlags(RF_Transactional);
					check(LODLevel->SpawnModule);
					LODLevel->SpawnModule->SetFlags(RF_Transactional);
					for (INT jj = 0; jj < LODLevel->Modules.Num(); jj++)
					{
						UParticleModule* pkModule = LODLevel->Modules(jj);
						pkModule->SetFlags(RF_Transactional);
					}
				}
			}
		}
	}

	// Nothing selected initially
	SelectedEmitter = NULL;
	SelectedModule = NULL;

	CopyModule = NULL;
	CopyEmitter = NULL;

	CurveToReplace = NULL;

	bResetOnFinish = TRUE;
	bPendingReset = FALSE;
	bOrbitMode = TRUE;
	bWireframe = FALSE;
	bBounds = FALSE;
	SimSpeed = 1.0f;

	bTransactionInProgress = FALSE;

	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create(this, this);

	// Create particle system preview
	WxCascadePreview* PreviewWindow = new WxCascadePreview( this, -1, this );
	PreviewVC = PreviewWindow->CascadePreviewVC;
	PreviewVC->SetPreviewCamera(PartSys->ThumbnailAngle, PartSys->ThumbnailDistance);
	PreviewVC->SetViewLocationForOrbiting( FVector(0.f,0.f,0.f) );
	if (EditorOptions->bShowGrid == TRUE)
	{
		PreviewVC->ShowFlags |= SHOW_Grid;
	}
	else
	{
		PreviewVC->ShowFlags &= ~SHOW_Grid;
	}

	PreviewVC->bDrawParticleCounts = EditorOptions->bShowParticleCounts;
	PreviewVC->bDrawParticleTimes = EditorOptions->bShowParticleTimes;
	PreviewVC->bDrawSystemDistance = EditorOptions->bShowParticleDistance;
	PreviewVC->bDrawParticleEvents = EditorOptions->bShowParticleEvents;

	UpdatePostProcessChain();

	// Create new curve editor setup if not already done
	if (!PartSys->CurveEdSetup)
	{
		PartSys->CurveEdSetup = ConstructObject<UInterpCurveEdSetup>( UInterpCurveEdSetup::StaticClass(), PartSys, NAME_None, RF_NotForClient | RF_NotForServer | RF_Transactional );
	}

	// Create graph editor to work on systems CurveEd setup.
	CurveEd = new WxCurveEditor( this, -1, PartSys->CurveEdSetup );
	// Register this window with the Curve editor so we will be notified of various things.
	CurveEd->SetNotifyObject(this);

	// Create emitter editor
	EmitterEdWindow = new WxCascadeEmitterEd(this, -1, this);
	EmitterEdVC = EmitterEdWindow->EmitterEdVC;

	// Create Docking Windows
	{
		AddDockingWindow(PropertyWindow, FDockingParent::DH_Bottom, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("PropertiesCaption_F")), *PartSys->GetName()) ), *LocalizeUnrealEd(TEXT("Properties")) );
		AddDockingWindow(CurveEd, FDockingParent::DH_Bottom, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("CurveEditorCaption_F")), *PartSys->GetName()) ), *LocalizeUnrealEd(TEXT("CurveEditor")) );
		AddDockingWindow(PreviewWindow, FDockingParent::DH_Left, *FString::Printf( LocalizeSecure(LocalizeUnrealEd(TEXT("PreviewCaption_F")), *PartSys->GetName()) ), *LocalizeUnrealEd(TEXT("Preview")) );
		
		SetDockHostSize(FDockingParent::DH_Left, 500);

		AddDockingWindow( EmitterEdWindow, FDockingParent::DH_None, NULL );

		// Try to load a existing layout for the docking windows.
		LoadDockingLayout();
	}

	// Create menu bar
	MenuBar = new WxCascadeMenuBar(this);
	AppendWindowMenu(MenuBar);
	SetMenuBar( MenuBar );

	// Create tool bar
	ToolBar	= NULL;
	ToolBar = new WxCascadeToolBar( this, -1 );
	SetToolBar( ToolBar );

	// Set window title to particle system we are editing.
	SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("CascadeCaption_F"), *PartSys->GetName()) ) );

	// Set emitter to use the particle system we are editing.
	PartSysComp->SetTemplate(PartSys);

	SetSelectedModule(NULL, NULL);

	PreviewVC->BackgroundColor = EditorOptions->BackgroundColor;

	// Setup the accelerator table
	TArray<wxAcceleratorEntry> Entries;
	// Allow derived classes an opportunity to register accelerator keys.
	// Bind SPACE to reset.
	if (EditorOptions->bUseSpaceBarResetInLevel == FALSE)
	{
		Entries.AddItem(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_SPACE, IDM_CASCADE_RESETSYSTEM));
	}
	else
	{
		Entries.AddItem(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_SPACE, IDM_CASCADE_RESETINLEVEL));
	}
	// Create the new table with these.
	SetAcceleratorTable(wxAcceleratorTable(Entries.Num(),Entries.GetTypedData()));

	PartSysComp->InitializeSystem();
	PartSysComp->ActivateSystem();

	UpdateLODLevelControls();
}

WxCascade::~WxCascade()
{
	GEditor->ResetTransaction(TEXT("QuitCascade"));

	// Save the desired window position to the .ini file
	FWindowUtil::SavePosSize(TEXT("CascadeEditor"), this);
	
	SaveDockingLayout();

	// Destroy preview viewport before we destroy the level.
	GEngine->Client->CloseViewport(PreviewVC->Viewport);
	PreviewVC->Viewport = NULL;

	if (PreviewVC->FloorComponent)
	{
		EditorOptions->FloorPosition = PreviewVC->FloorComponent->Translation;
		EditorOptions->FloorRotation = PreviewVC->FloorComponent->Rotation;
		EditorOptions->FloorScale = PreviewVC->FloorComponent->Scale;
		EditorOptions->FloorScale3D = PreviewVC->FloorComponent->Scale3D;

		if (PreviewVC->FloorComponent->StaticMesh)
		{
			if (PreviewVC->FloorComponent->StaticMesh->GetOuter())
			{
				EditorOptions->FloorMesh = PreviewVC->FloorComponent->StaticMesh->GetOuter()->GetName();
				EditorOptions->FloorMesh += TEXT(".");
			}
			else
			{
				warnf(TEXT("Unable to locate Cascade floor mesh outer..."));
				EditorOptions->FloorMesh = TEXT("");
			}

			EditorOptions->FloorMesh += PreviewVC->FloorComponent->StaticMesh->GetName();
		}
		else
		{
			EditorOptions->FloorMesh += FString::Printf(TEXT("EditorMeshes.AnimTreeEd_PreviewFloor"));
		}

		FString	Name;

		Name = EditorOptions->FloorMesh;
		debugf(TEXT("StaticMesh       = %s"), *Name);

		EditorOptions->SaveConfig();
	}

	delete PreviewVC;
	PreviewVC = NULL;

	delete PropertyWindow;
}

/**
 * This function is called when the window has been selected from within the ctrl + tab dialog.
 */
void WxCascade::OnSelected()
{
	Raise();
}

wxToolBar* WxCascade::OnCreateToolBar(long style, wxWindowID id, const wxString& name)
{
	if (name == TEXT("Cascade"))
		return new WxCascadeToolBar(this, -1);

	wxToolBar*	ReturnToolBar = OnCreateToolBar(style, id, name);
	if (ReturnToolBar)
	{
		UpdateLODLevelControls();
	}

	return ReturnToolBar;
}

void WxCascade::Serialize(FArchive& Ar)
{
	PreviewVC->Serialize(Ar);

	Ar << EditorOptions;
}

/**
 * Pure virtual that must be overloaded by the inheriting class. It will
 * be called from within UnLevTick.cpp after ticking all actors.
 *
 * @param DeltaTime	Game time passed since the last call.
 */
static const DOUBLE ResetInterval = 0.5f;
void WxCascade::Tick( FLOAT DeltaTime )
{
	if(GIsPlayInEditorWorld)
	{
		return;
	}

	// Don't bother ticking at all if paused.
	if (PreviewVC->TimeScale > KINDA_SMALL_NUMBER)
	{
		const FLOAT fSaveUpdateDelta = PartSys->UpdateTime_Delta;
		if (PreviewVC->TimeScale < 1.0f)
		{
			PartSys->UpdateTime_Delta *= PreviewVC->TimeScale;
		}

		PartSysComp->Tick(PreviewVC->TimeScale * DeltaTime);

		PreviewVC->TotalTime += PreviewVC->TimeScale * DeltaTime;

		PartSys->UpdateTime_Delta = fSaveUpdateDelta;
	}

	// If we are doing auto-reset
	if(bResetOnFinish)
	{
		UParticleSystemComponent* PartComp = PartSysComp;

		// If system has finish, pause for a bit before resetting.
		if(bPendingReset)
		{
			if(PreviewVC->TotalTime > ResetTime)
			{
				PartComp->ResetParticles();
				PartComp->ActivateSystem();

				bPendingReset = FALSE;
			}
		}
		else
		{
			if( PartComp->HasCompleted() )
			{
				bPendingReset = TRUE;
				ResetTime = PreviewVC->TotalTime + ResetInterval;
			}
		}
	}
}

// FCurveEdNotifyInterface
/**
 *	PreEditCurve
 *	Called by the curve editor when N curves are about to change
 *
 *	@param	CurvesAboutToChange		An array of the curves about to change
 */
void WxCascade::PreEditCurve(TArray<UObject*> CurvesAboutToChange)
{
	debugf(TEXT("CASCADE: PreEditCurve - %2d curves"), CurvesAboutToChange.Num());

//	InterpEdTrans->BeginSpecial(*LocalizeUnrealEd("CurveEdit"));
	BeginTransaction(*LocalizeUnrealEd("CurveEdit"));
	ModifyParticleSystem();
	ModifySelectedObjects();

	// Call Modify on all tracks with keys selected
	for (INT i = 0; i < CurvesAboutToChange.Num(); i++)
	{
		// If this keypoint is from a distribution, call Modify on it to back up its state.
		UDistributionFloat* DistFloat = Cast<UDistributionFloat>(CurvesAboutToChange(i));
		if (DistFloat)
		{
			DistFloat->SetFlags(RF_Transactional);
			DistFloat->Modify();
		}
		UDistributionVector* DistVector = Cast<UDistributionVector>(CurvesAboutToChange(i));
		if (DistVector)
		{
			DistVector->SetFlags(RF_Transactional);
			DistVector->Modify();
		}
	}
}

/**
 *	PostEditCurve
 *	Called by the curve editor when the edit has completed.
 */
void WxCascade::PostEditCurve()
{
	debugf(TEXT("CASCADE: PostEditCurve"));
//	InterpEdTrans->EndSpecial();
	this->EndTransaction(*LocalizeUnrealEd("CurveEdit"));
}

/**
 *	MovedKey
 *	Called by the curve editor when a key has been moved.
 */
void WxCascade::MovedKey()
{
}

/**
 *	DesireUndo
 *	Called by the curve editor when an Undo is requested.
 */
void WxCascade::DesireUndo()
{
	debugf(TEXT("CASCADE: DesireUndo"));
//	InterpEdUndo();
	CascadeUndo();
}

/**
 *	DesireRedo
 *	Called by the curve editor when an Redo is requested.
 */
void WxCascade::DesireRedo()
{
	debugf(TEXT("CASCADE: DesireRedo"));
//	InterpEdRedo();
	CascadeRedo();
}

void WxCascade::OnSize( wxSizeEvent& In )
{
	In.Skip();
	Refresh();
}

///////////////////////////////////////////////////////////////////////////////////////
// Menu Callbacks

void WxCascade::OnRenameEmitter(wxCommandEvent& In)
{
	if (!SelectedEmitter)
		return;

	BeginTransaction(TEXT("EmitterRename"));

	PartSys->PreEditChange(NULL);
	PartSysComp->PreEditChange(NULL);

	FName& CurrentName = SelectedEmitter->GetEmitterName();

	WxDlgGenericStringEntry dlg;
	if (dlg.ShowModal(TEXT("RenameEmitter"), TEXT("Name"), *CurrentName.ToString()) == wxID_OK)
	{
		FName newName = FName(*(dlg.GetEnteredString()));
		SelectedEmitter->SetEmitterName(newName);
	}

	PartSysComp->PostEditChange(NULL);
	PartSys->PostEditChange(NULL);

	EndTransaction(TEXT("EmitterRename"));

	// Refresh viewport
	EmitterEdVC->Viewport->Invalidate();
}

void WxCascade::OnNewEmitter(wxCommandEvent& In)
{
	BeginTransaction(TEXT("NewEmitter"));
	PartSys->PreEditChange(NULL);
	PartSysComp->PreEditChange(NULL);

	UClass* NewEmitClass = UParticleSpriteEmitter::StaticClass();

	// Construct it
	UParticleEmitter* NewEmitter = ConstructObject<UParticleEmitter>(NewEmitClass, PartSys, NAME_None, RF_Transactional);
	UParticleLODLevel* LODLevel	= NewEmitter->GetLODLevel(0);
	if (LODLevel == NULL)
	{
		// Generate the HighLOD level, and the default lowest level
		INT Index = NewEmitter->CreateLODLevel(0);
		LODLevel = NewEmitter->GetLODLevel(0);
	}

	check(LODLevel);

	LODLevel->RequiredModule->EmitterEditorColor	= FColor::MakeRandomColor();
	
    // Set to sensible default values
	NewEmitter->SetToSensibleDefaults();

    // Handle special cases...
	if (NewEmitClass == UParticleSpriteEmitter::StaticClass())
	{
		// For handyness- use currently selected material for new emitter (or default if none selected)
		UParticleSpriteEmitter* NewSpriteEmitter = (UParticleSpriteEmitter*)NewEmitter;
		UMaterialInterface* CurrentMaterial = GEditor->GetSelectedObjects()->GetTop<UMaterialInterface>();
		if (CurrentMaterial)
		{
			LODLevel->RequiredModule->Material = CurrentMaterial;
		}
		else
		{
			LODLevel->RequiredModule->Material = LoadObject<UMaterialInterface>(NULL, TEXT("EngineMaterials.DefaultParticle"), NULL, LOAD_None, NULL);
		}
	}

	if (NewEmitter->AutogenerateLowestLODLevel(PartSys->bRegenerateLODDuplicate) == FALSE)
	{
		warnf(TEXT("Failed to autogenerate lowest LOD level!"));
	}

	// Generate all the levels that are present in other emitters...
	if (PartSys->Emitters.Num() > 0)
	{
		UParticleEmitter* ExistingEmitter = PartSys->Emitters(0);
		if (ExistingEmitter->LODLevels.Num() > 2)
		{
			debugf(TEXT("Generating existing LOD levels..."));

			// Walk the LOD levels of the existing emitter...
			UParticleLODLevel* ExistingLOD;
			UParticleLODLevel* NewLOD_Prev = NewEmitter->LODLevels(0);
			UParticleLODLevel* NewLOD_Next = NewEmitter->LODLevels(1);

			check(NewLOD_Prev);
			check(NewLOD_Next);

			for (INT LODIndex = 1; LODIndex < ExistingEmitter->LODLevels.Num() - 1; LODIndex++)
			{
				ExistingLOD = ExistingEmitter->LODLevels(LODIndex);

				// Add this one
				INT ResultIndex = NewEmitter->CreateLODLevel(ExistingLOD->Level, TRUE);

				UParticleLODLevel* NewLODLevel	= NewEmitter->LODLevels(ResultIndex);
				check(NewLODLevel);
				NewLODLevel->UpdateModuleLists();
			}
		}
	}

	NewEmitter->UpdateModuleLists();

	NewEmitter->PostEditChange(NULL);
	if (NewEmitter)
	{
		NewEmitter->SetFlags(RF_Transactional);
		for (INT LODIndex = 0; LODIndex < NewEmitter->LODLevels.Num(); LODIndex++)
		{
			UParticleLODLevel* LODLevel = NewEmitter->GetLODLevel(LODIndex);
			if (LODLevel)
			{
				LODLevel->SetFlags(RF_Transactional);
				check(LODLevel->RequiredModule);
				LODLevel->RequiredModule->SetFlags(RF_Transactional);
				check(LODLevel->SpawnModule);
				LODLevel->SpawnModule->SetFlags(RF_Transactional);
				for (INT jj = 0; jj < LODLevel->Modules.Num(); jj++)
				{
					UParticleModule* pkModule = LODLevel->Modules(jj);
					pkModule->SetFlags(RF_Transactional);
				}
			}
		}
	}

    // Add to selected emitter
    PartSys->Emitters.AddItem(NewEmitter);

	// Setup the LOD distances
	if (PartSys->LODDistances.Num() == 0)
	{
		UParticleEmitter* Emitter = PartSys->Emitters(0);
		if (Emitter)
		{
			PartSys->LODDistances.Add(Emitter->LODLevels.Num());
			for (INT LODIndex = 0; LODIndex < PartSys->LODDistances.Num(); LODIndex++)
			{
				PartSys->LODDistances(LODIndex) = LODIndex * 2500.0f;
			}
		}
	}

	PartSysComp->PostEditChange(NULL);
	PartSys->PostEditChange(NULL);

	EndTransaction(TEXT("NewEmitter"));

	// Refresh viewport
	EmitterEdVC->Viewport->Invalidate();
}

void WxCascade::OnSelectParticleSystem( wxCommandEvent& In )
{
	SetSelectedEmitter(NULL);
}

void WxCascade::OnNewEmitterBefore( wxCommandEvent& In )
{
	if ((SelectedEmitter != NULL) && (PartSys != NULL))
	{
		INT EmitterCount = PartSys->Emitters.Num();
		INT EmitterIndex = -1;
		for (INT Index = 0; Index < EmitterCount; Index++)
		{
			UParticleEmitter* CheckEmitter = PartSys->Emitters(Index);
			if (SelectedEmitter == CheckEmitter)
			{
				EmitterIndex = Index;
				break;
			}
		}

		if (EmitterIndex != -1)
		{
			debugf(TEXT("Insert New Emitter Before %d"), EmitterIndex);

			// Fake create it at the end
			wxCommandEvent DummyIn;
			DummyIn.SetId(IDM_CASCADE_NEW_EMITTER_START);
			OnNewEmitter(DummyIn);

			if (EmitterCount + 1 == PartSys->Emitters.Num())
			{
				UParticleEmitter* NewEmitter = PartSys->Emitters(EmitterCount);
				SetSelectedEmitter(NewEmitter);
				MoveSelectedEmitter(EmitterIndex - EmitterCount);
			}
		}
	}
}

void WxCascade::OnNewEmitterAfter( wxCommandEvent& In )
{
	if ((SelectedEmitter != NULL) && (PartSys != NULL))
	{
		INT EmitterCount = PartSys->Emitters.Num();
		INT EmitterIndex = -1;
		for (INT Index = 0; Index < EmitterCount; Index++)
		{
			UParticleEmitter* CheckEmitter = PartSys->Emitters(Index);
			if (SelectedEmitter == CheckEmitter)
			{
				EmitterIndex = Index;
				break;
			}
		}

		if (EmitterIndex != -1)
		{
			debugf(TEXT("Insert New Emitter After  %d"), EmitterIndex);

			// Fake create it at the end
			wxCommandEvent DummyIn;
			DummyIn.SetId(IDM_CASCADE_NEW_EMITTER_START);
			OnNewEmitter(DummyIn);

			if (EmitterCount + 1 == PartSys->Emitters.Num())
			{
				UParticleEmitter* NewEmitter = PartSys->Emitters(EmitterCount);
				SetSelectedEmitter(NewEmitter);
				if (EmitterIndex + 1 < EmitterCount)
				{
					MoveSelectedEmitter(EmitterIndex - EmitterCount + 1);
				}
			}
		}
	}
}

void WxCascade::OnNewModule(wxCommandEvent& In)
{
	if (!SelectedEmitter)
		return;

	// Find desired class of new module.
	INT NewModClassIndex = In.GetId() - IDM_CASCADE_NEW_MODULE_START;
	check( NewModClassIndex >= 0 && NewModClassIndex < ParticleModuleClasses.Num() );

	CreateNewModule(NewModClassIndex);
}

void WxCascade::OnDuplicateEmitter(wxCommandEvent& In)
{
	// Make sure there is a selected emitter
	if (!SelectedEmitter)
		return;

	UBOOL bShare = FALSE;
	if (In.GetId() == IDM_CASCADE_DUPLICATE_SHARE_EMITTER)
	{
		bShare = TRUE;
	}

	BeginTransaction(TEXT("EmitterDuplicate"));

	PartSys->PreEditChange(NULL);
	PartSysComp->PreEditChange(NULL);

	if (!DuplicateEmitter(SelectedEmitter, PartSys, bShare))
	{
	}
	PartSysComp->PostEditChange(NULL);
	PartSys->PostEditChange(NULL);

	EndTransaction(TEXT("EmitterDuplicate"));

	// Refresh viewport
	EmitterEdVC->Viewport->Invalidate();
}

void WxCascade::OnDeleteEmitter(wxCommandEvent& In)
{
	DeleteSelectedEmitter();
}

void WxCascade::OnExportEmitter(wxCommandEvent& In)
{
	ExportSelectedEmitter();
}

void WxCascade::OnConvertRainEmitter(wxCommandEvent& In)
{
	check(GIsEpicInternal);

	if (!SelectedEmitter)
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("Error_MustSelectEmitter"));
		return;
	}

	if (appMsgf(AMT_YesNo, *LocalizeUnrealEd("UberModuleConvertConfirm")))
	{
		BeginTransaction(TEXT("EmitterUberRainConvert"));

		// Determine which Rain module is suitable
		UParticleModuleUberRainDrops*	RainDrops	= ConstructObject<UParticleModuleUberRainDrops>(UParticleModuleUberRainDrops::StaticClass(), SelectedEmitter->GetOuter(), NAME_None, RF_Transactional);
		check(RainDrops);
		UParticleModuleUberRainImpacts* RainImpacts	= ConstructObject<UParticleModuleUberRainImpacts>(UParticleModuleUberRainImpacts::StaticClass(), SelectedEmitter->GetOuter(), NAME_None, RF_Transactional);
		check(RainImpacts);
		UParticleModuleUberRainSplashA* RainSplashA	= ConstructObject<UParticleModuleUberRainSplashA>(UParticleModuleUberRainSplashA::StaticClass(), SelectedEmitter->GetOuter(), NAME_None, RF_Transactional);
		check(RainSplashA);
		UParticleModuleUberRainSplashB* RainSplashB	= ConstructObject<UParticleModuleUberRainSplashB>(UParticleModuleUberRainSplashB::StaticClass(), SelectedEmitter->GetOuter(), NAME_None, RF_Transactional);
		check(RainSplashB);

		UParticleModuleUberBase* RainBase	= NULL;

		if (RainBase == NULL && RainDrops->IsCompatible(SelectedEmitter))
		{
			RainBase = Cast<UParticleModuleUberBase>(RainDrops);
		}

		if (RainBase == NULL && RainImpacts->IsCompatible(SelectedEmitter))
		{
			RainBase = Cast<UParticleModuleUberBase>(RainImpacts);
		}
	
		if (RainBase == NULL && RainSplashA->IsCompatible(SelectedEmitter))
		{
			RainBase = Cast<UParticleModuleUberBase>(RainSplashA);
		}
	
		if (RainBase == NULL && RainSplashB->IsCompatible(SelectedEmitter))
		{
			RainBase = Cast<UParticleModuleUberBase>(RainSplashB);
		}
		
		if (RainBase)
		{
			// First, duplicate the emitter to have a back-up...
			PartSys->PreEditChange(NULL);
			PartSysComp->PreEditChange(NULL);

			UParticleLODLevel* LODLevel = SelectedEmitter->LODLevels(0);
			UBOOL bEnabledSave	= LODLevel->bEnabled;
			LODLevel->bEnabled = FALSE;
			if (!DuplicateEmitter(SelectedEmitter, PartSys))
			{
				check(!TEXT("BAD DUPLICATE IN UBER-RAIN CONVERSION!"));
			}
			LODLevel->bEnabled = bEnabledSave;

			// Convert it
			if (RainBase->ConvertToUberModule(SelectedEmitter) == TRUE)
			{
				// DID IT!
			}
			else
			{
				appMsgf(AMT_OK, *LocalizeUnrealEd("Error_FailedToConverToUberModule"));
			}

			PartSysComp->PostEditChange(NULL);
			PartSys->PostEditChange(NULL);
		}
		else
		{
			appMsgf(AMT_OK, *LocalizeUnrealEd("Error_FailedToFindUberModule"));
		}

		EndTransaction(TEXT("EmitterUberRainConvert"));

		wxCommandEvent DummyEvent;
		OnRegenerateLowestLOD( DummyEvent );

		// Mark package as dirty
		SelectedEmitter->MarkPackageDirty();

		// Redraw the module window
		EmitterEdVC->Viewport->Invalidate();
	}
}

void WxCascade::OnAddSelectedModule(wxCommandEvent& In)
{
}

void WxCascade::OnCopyModule(wxCommandEvent& In)
{
	if (SelectedModule)
		SetCopyModule(SelectedEmitter, SelectedModule);
}

void WxCascade::OnPasteModule(wxCommandEvent& In)
{
	if (!CopyModule)
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("Prompt_5"));
		return;
	}

	if (SelectedEmitter && CopyEmitter && (SelectedEmitter == CopyEmitter))
	{
		// Can't copy onto ourselves... Or can we
		appMsgf(AMT_OK, *LocalizeUnrealEd("Prompt_6"));
		return;
	}

	PasteCurrentModule();
}

void WxCascade::OnDeleteModule(wxCommandEvent& In)
{
	DeleteSelectedModule();
}

void WxCascade::OnEnableModule(wxCommandEvent& In)
{
	EnableSelectedModule();
}

void WxCascade::OnResetModule(wxCommandEvent& In)
{
	ResetSelectedModule();
}

void WxCascade::OnRefreshModule(wxCommandEvent& In)
{
	RefreshSelectedModule();
}

/** Sync the sprite material in the generic browser */
void WxCascade::OnModuleSyncMaterial( wxCommandEvent& In )
{
	TArray<UObject*> Objects;

	if (SelectedModule)
	{
		UParticleModuleRequired* RequiredModule = Cast<UParticleModuleRequired>(SelectedModule);
		if (RequiredModule)
		{
			Objects.AddItem(RequiredModule->Material);
		}
	}

	// Sync the generic browser to the object list.
	if ( Objects.Num() > 0 )
	{
		WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
		if ( GenericBrowser )
		{
			// Make sure the window is visible.  The window needs to be visible *before*
			// the browser is sync'd to objects so that redraws actually happen!
			GUnrealEd->GetBrowserManager()->ShowWindow( GenericBrowser->GetDockID(), TRUE );

			// Sync.
			GenericBrowser->SyncToObjects( Objects );
		}
	}
}

/** Assign the selected material to the sprite material */
void WxCascade::OnModuleUseMaterial( wxCommandEvent& In )
{
	if (SelectedModule && SelectedEmitter)
	{
		UParticleModuleRequired* RequiredModule = Cast<UParticleModuleRequired>(SelectedModule);
		if (RequiredModule)
		{
			UObject* Obj = GEditor->GetSelectedObjects()->GetTop(UMaterialInterface::StaticClass());
			if (Obj)
			{
				UMaterialInterface* SelectedMaterial = Cast<UMaterialInterface>(Obj);
				if (SelectedMaterial)
				{
					RequiredModule->Material = SelectedMaterial;
					SelectedEmitter->PostEditChange(NULL);
				}
			}
		}
	}
}

void WxCascade::OnModuleDupHighest( wxCommandEvent& In )
{
	DupHighestSelectedModule();
}

void WxCascade::OnModuleDupHigher( wxCommandEvent& In )
{
	DupHigherSelectedModule();
}

/**
 *	Set the module to the SAME module in the next higher LOD level.
 */
void WxCascade::OnModuleShareHigher( wxCommandEvent& In )
{
	ShareHigherSelectedModule();
}

void WxCascade::OnMenuSimSpeed(wxCommandEvent& In)
{
	INT Id = In.GetId();

	if (Id == IDM_CASCADE_SIM_PAUSE)
	{
		PreviewVC->TimeScale = 0.f;
		ToolBar->ToggleTool(IDM_CASCADE_PLAY, FALSE);
		ToolBar->ToggleTool(IDM_CASCADE_PAUSE, TRUE);
	}
	else
	{
		if ((Id == IDM_CASCADE_SIM_1PERCENT) || 
			(Id == IDM_CASCADE_SIM_10PERCENT) || 
			(Id == IDM_CASCADE_SIM_25PERCENT) || 
			(Id == IDM_CASCADE_SIM_50PERCENT) || 
			(Id == IDM_CASCADE_SIM_NORMAL))
		{
			ToolBar->ToggleTool(IDM_CASCADE_PLAY, TRUE);
			ToolBar->ToggleTool(IDM_CASCADE_PAUSE, FALSE);
		}

		if (Id == IDM_CASCADE_SIM_1PERCENT)
		{
			PreviewVC->TimeScale = 0.01f;
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_1, TRUE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_10, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_25, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_50, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_100, FALSE);
		}
		else if (Id == IDM_CASCADE_SIM_10PERCENT)
		{
			PreviewVC->TimeScale = 0.1f;
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_1, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_10, TRUE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_25, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_50, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_100, FALSE);
		}
		else if (Id == IDM_CASCADE_SIM_25PERCENT)
		{
			PreviewVC->TimeScale = 0.25f;
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_1, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_10, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_25, TRUE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_50, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_100, FALSE);
		}
		else if (Id == IDM_CASCADE_SIM_50PERCENT)
		{
			PreviewVC->TimeScale = 0.5f;
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_1, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_10, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_25, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_50, TRUE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_100, FALSE);
		}
		else if (Id == IDM_CASCADE_SIM_NORMAL)
		{
			PreviewVC->TimeScale = 1.f;
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_1, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_10, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_25, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_50, FALSE);
			ToolBar->ToggleTool(IDM_CASCADE_SPEED_100, TRUE);
		}
	}
}

void WxCascade::OnSaveCam(wxCommandEvent& In)
{
	PartSys->ThumbnailAngle = PreviewVC->PreviewAngle;
	PartSys->ThumbnailDistance = PreviewVC->PreviewDistance;
	PartSys->PreviewComponent = NULL;

	PreviewVC->bCaptureScreenShot = TRUE;
}

void WxCascade::OnResetSystem(wxCommandEvent& In)
{
	if (PartSysComp)
	{
		PartSysComp->ResetParticles();
		PartSysComp->ActivateSystem();
		PartSysComp->Template->bShouldResetPeakCounts = TRUE;
		if (PreviewVC)
		{
			PreviewVC->PreviewScene.RemoveComponent(PartSysComp);
			PreviewVC->PreviewScene.AddComponent(PartSysComp, FMatrix::Identity);
		}
	}

	if (PartSys)
	{
		PartSys->CalculateMaxActiveParticleCounts();
	}
}

void WxCascade::OnResetInLevel(wxCommandEvent& In)
{
	OnResetSystem(In);
	for (TObjectIterator<UParticleSystemComponent> It;It;++It)
	{
		if (It->Template == PartSysComp->Template)
		{
			UParticleSystemComponent* PSysComp = *It;

			// Force a recache of the view relevance
			PSysComp->bIsViewRelevanceDirty = TRUE;

			PSysComp->ResetParticles();
			PSysComp->ActivateSystem();
			PSysComp->Template->bShouldResetPeakCounts = TRUE;

			FSceneInterface* Scene = PSysComp->GetScene();
			if (Scene)
			{
				Scene->RemovePrimitive(PSysComp);
				Scene->AddPrimitive(PSysComp);
			}
		}
	}
}

void WxCascade::OnResetPeakCounts(wxCommandEvent& In)
{
	PartSysComp->ResetParticles();
/***
	for (INT i = 0; i < PartSysComp->Template->Emitters.Num(); i++)
	{
		UParticleEmitter* Emitter = PartSysComp->Template->Emitters(i);
		for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
		{
			UParticleLODLevel* LODLevel = Emitter->LODLevels(LODIndex);
			LODLevel->PeakActiveParticles = 0;
		}
	}
***/
	PartSysComp->Template->bShouldResetPeakCounts = TRUE;
	PartSysComp->InitParticles();
}

void WxCascade::OnUberConvert(wxCommandEvent& In)
{
	if (!SelectedEmitter)
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("Error_MustSelectEmitter"));
		return;
	}

	if (appMsgf(AMT_YesNo, *LocalizeUnrealEd("UberModuleConvertConfirm")))
	{
		BeginTransaction(TEXT("EmitterUberConvert"));

		// Find the best uber module
		UParticleModuleUberBase* UberModule	= Cast<UParticleModuleUberBase>(
			UParticleModuleUberBase::DetermineBestUberModule(SelectedEmitter));
		if (!UberModule)
		{
			appMsgf(AMT_OK, *LocalizeUnrealEd("Error_FailedToFindUberModule"));
			EndTransaction(TEXT("EmitterUberConvert"));
			return;
		}

		// Convert it
		if (UberModule->ConvertToUberModule(SelectedEmitter) == FALSE)
		{
			appMsgf(AMT_OK, *LocalizeUnrealEd("Error_FailedToConverToUberModule"));
			EndTransaction(TEXT("EmitterUberConvert"));
			return;
		}

		EndTransaction(TEXT("EmitterUberConvert"));

		// Mark package as dirty
		SelectedEmitter->MarkPackageDirty();

		// Redraw the module window
		EmitterEdVC->Viewport->Invalidate();
	}
}

/**
 *	OnRegenerateLowestLOD
 *	This function is supplied to remove all LOD levels and regenerate the lowest.
 *	It is intended for use once the artists/designers decide on a suitable baseline
 *	for the lowest LOD generation parameters.
 */
void WxCascade::OnRegenerateLowestLOD(wxCommandEvent& In)
{
	if ((PartSys == NULL) || (PartSys->Emitters.Num() == 0))
	{
		return;
	}

	PartSys->bRegenerateLODDuplicate = FALSE;

	FString	WarningMessage(TEXT(""));

	WarningMessage += *LocalizeUnrealEd("CascadeRegenLowLODWarningLine1");
	WarningMessage += TEXT("\n");
	WarningMessage += *LocalizeUnrealEd("CascadeRegenLowLODWarningLine2");
	WarningMessage += TEXT("\n");
	WarningMessage += *LocalizeUnrealEd("CascadeRegenLowLODWarningLine3");
	WarningMessage += TEXT("\n");
	WarningMessage += *LocalizeUnrealEd("CascadeRegenLowLODWarningLine4");

	if (appMsgf(AMT_YesNo, *WarningMessage) == TRUE)
	{
		debugf(TEXT("Regenerate Lowest LOD levels!"));

		BeginTransaction(TEXT("CascadeRegenerateLowestLOD"));
		ModifyParticleSystem();

		// Delete all LOD levels from each emitter...
		for (INT EmitterIndex = 0; EmitterIndex < PartSys->Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter*	Emitter	= PartSys->Emitters(EmitterIndex);
			if (Emitter)
			{
				for (INT LODIndex = Emitter->LODLevels.Num() - 1; LODIndex > 0; LODIndex--)
				{
					Emitter->LODLevels.Remove(LODIndex);
				}
				if (Emitter->AutogenerateLowestLODLevel(PartSys->bRegenerateLODDuplicate) == FALSE)
				{
					warnf(TEXT("Failed to autogenerate lowest LOD level!"));
				}

				Emitter->UpdateModuleLists();
			}
		}

		// Reset the LOD distances
		PartSys->LODDistances.Empty();
		UParticleEmitter* SourceEmitter = PartSys->Emitters(0);
		if (SourceEmitter)
		{
			PartSys->LODDistances.Add(SourceEmitter->LODLevels.Num());
			for (INT LODIndex = 0; LODIndex < PartSys->LODDistances.Num(); LODIndex++)
			{
				PartSys->LODDistances(LODIndex) = LODIndex * 2500.0f;
			}
		}

		check(TransactionInProgress());
		EndTransaction(TEXT("CascadeRegenerateLowestLOD"));

		// Re-fill the LODCombo so that deleted LODLevels are removed.
		EmitterEdVC->Viewport->Invalidate();
		PropertyWindow->Rebuild();

		wxCommandEvent DummyEvent;
		OnResetInLevel(DummyEvent);
	}
	else
	{
		debugf(TEXT("CANCELLED Regenerate Lowest LOD levels!"));
	}

	UpdateLODLevelControls();
}

/**
 *	OnRegenerateLowestLODDuplicateHighest
 *	This function is supplied to remove all LOD levels and regenerate the lowest.
 *	It is intended for use once the artists/designers decide on a suitable baseline
 *	for the lowest LOD generation parameters.
 *	It will duplicate the highest LOD as the lowest.
 */
void WxCascade::OnRegenerateLowestLODDuplicateHighest(wxCommandEvent& In)
{
	if ((PartSys == NULL) || (PartSys->Emitters.Num() == 0))
	{
		return;
	}

	PartSys->bRegenerateLODDuplicate = TRUE;

	FString	WarningMessage(TEXT(""));

	WarningMessage += *LocalizeUnrealEd("CascadeRegenLowLODWarningLine1");
	WarningMessage += TEXT("\n");
	WarningMessage += *LocalizeUnrealEd("CascadeRegenLowLODWarningLine2");
	WarningMessage += TEXT("\n");
	WarningMessage += *LocalizeUnrealEd("CascadeRegenLowLODWarningLine3");
	WarningMessage += TEXT("\n");
	WarningMessage += *LocalizeUnrealEd("CascadeRegenLowLODWarningLine4");

	if (appMsgf(AMT_YesNo, *WarningMessage) == TRUE)
	{
		debugf(TEXT("Regenerate Lowest LOD levels!"));

		BeginTransaction(TEXT("CascadeRegenerateLowestLOD"));
		ModifyParticleSystem();

		// Delete all LOD levels from each emitter...
		for (INT EmitterIndex = 0; EmitterIndex < PartSys->Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter*	Emitter	= PartSys->Emitters(EmitterIndex);
			if (Emitter)
			{
				for (INT LODIndex = Emitter->LODLevels.Num() - 1; LODIndex > 0; LODIndex--)
				{
					Emitter->LODLevels.Remove(LODIndex);
				}
				if (Emitter->AutogenerateLowestLODLevel(PartSys->bRegenerateLODDuplicate) == FALSE)
				{
					warnf(TEXT("Failed to autogenerate lowest LOD level!"));
				}

				Emitter->UpdateModuleLists();
			}
		}

		// Reset the LOD distances
		PartSys->LODDistances.Empty();
		UParticleEmitter* SourceEmitter = PartSys->Emitters(0);
		if (SourceEmitter)
		{
			PartSys->LODDistances.Add(SourceEmitter->LODLevels.Num());
			for (INT LODIndex = 0; LODIndex < PartSys->LODDistances.Num(); LODIndex++)
			{
				PartSys->LODDistances(LODIndex) = LODIndex * 2500.0f;
			}
		}

		wxCommandEvent DummyEvent;
		OnResetInLevel(DummyEvent);

		check(TransactionInProgress());
		EndTransaction(TEXT("CascadeRegenerateLowestLOD"));

		// Re-fill the LODCombo so that deleted LODLevels are removed.
		EmitterEdVC->Viewport->Invalidate();
		PropertyWindow->Rebuild();
		if (PartSysComp)
		{
			PartSysComp->ResetParticles();
			PartSysComp->InitializeSystem();
		}
	}
	else
	{
		debugf(TEXT("CANCELLED Regenerate Lowest LOD levels!"));
	}

	UpdateLODLevelControls();
}

void WxCascade::OnSavePackage(wxCommandEvent& In)
{
	debugf(TEXT("SAVE PACKAGE"));
	if (!PartSys)
	{
		appMsgf(AMT_OK, TEXT("No particle system active..."));
		return;
	}

	UPackage* Package = Cast<UPackage>(PartSys->GetOutermost());
	if (Package)
	{
		debugf(TEXT("Have a package!"));

		FString FileTypes( TEXT("All Files|*.*") );
		
		for (INT i=0; i<GSys->Extensions.Num(); i++)
		{
			FileTypes += FString::Printf( TEXT("|(*.%s)|*.%s"), *GSys->Extensions(i), *GSys->Extensions(i) );
		}

		if (FindObject<UWorld>(Package, TEXT("TheWorld")))
		{
			appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_CantSaveMapViaCascade"), *Package->GetName()));
		}
		else
		{
			FString ExistingFile, File, Directory;
			FString PackageName = Package->GetName();

			if (GPackageFileCache->FindPackageFile( *PackageName, NULL, ExistingFile ))
			{
				FString Filename, Extension;
				GPackageFileCache->SplitPath( *ExistingFile, Directory, Filename, Extension );
				File = FString::Printf( TEXT("%s.%s"), *Filename, *Extension );
			}
			else
			{
				Directory = TEXT("");
				File = FString::Printf( TEXT("%s.upk"), *PackageName );
			}

			WxFileDialog SaveFileDialog( this, 
				*LocalizeUnrealEd("SavePackage"), 
				*Directory,
				*File,
				*FileTypes,
				wxSAVE,
				wxDefaultPosition);

			FString SaveFileName;

			if ( SaveFileDialog.ShowModal() == wxID_OK )
			{
				SaveFileName = FString( SaveFileDialog.GetPath() );

				if ( GFileManager->IsReadOnly( *SaveFileName ) || !GUnrealEd->Exec( *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\""), *PackageName, *SaveFileName) ) )
				{
					appMsgf( AMT_OK, *LocalizeUnrealEd("Error_CouldntSavePackage") );
				}
			}
		}

		if (PartSys)
		{
			PartSys->PostEditChange(NULL);
		}
	}
}

void WxCascade::OnOrbitMode(wxCommandEvent& In)
{
	bOrbitMode = In.IsChecked();

	//@todo. actually handle this...
	if (bOrbitMode)
	{
		PreviewVC->SetPreviewCamera(PreviewVC->PreviewAngle, PreviewVC->PreviewDistance);
	}
}

void WxCascade::OnWireframe(wxCommandEvent& In)
{
	bWireframe = In.IsChecked();
	PreviewVC->bWireframe = bWireframe;
}

void WxCascade::OnBounds(wxCommandEvent& In)
{
	bBounds = In.IsChecked();
	PreviewVC->bBounds = bBounds;
}

void WxCascade::OnBoundsRightClick(wxCommandEvent& In)
{
	if ((PartSys != NULL) && (PartSysComp != NULL))
	{
		if (appMsgf(AMT_YesNo, *LocalizeUnrealEd("Casc_SetFixedBounds")))
		{
			BeginTransaction(TEXT("CascadeSetFixedBounds"));

			// Grab the current bounds of the PSysComp & set it on the PSystem itself
			PartSys->FixedRelativeBoundingBox.Min = PartSysComp->Bounds.GetBoxExtrema(0);
			PartSys->FixedRelativeBoundingBox.Max = PartSysComp->Bounds.GetBoxExtrema(1);
			PartSys->FixedRelativeBoundingBox.IsValid = TRUE;
			PartSys->bUseFixedRelativeBoundingBox = TRUE;

			PartSys->MarkPackageDirty();

			EndTransaction(TEXT("CascadeSetFixedBounds"));

			PropertyWindow->Update();
		}
	}
}

/**
 *	Handler for turning post processing on and off.
 *
 *	@param	In	wxCommandEvent
 */
void WxCascade::OnPostProcess(wxCommandEvent& In)
{
	WxCascadePostProcessMenu* menu = new WxCascadePostProcessMenu(this);
	if (menu)
	{
		FTrackPopupMenu tpm(this, menu);
		tpm.Show();
		delete menu;
	}
}

/**
 *	Handler for turning the grid on and off.
 *
 *	@param	In	wxCommandEvent
 */
void WxCascade::OnToggleGrid(wxCommandEvent& In)
{
	bool bShowGrid = In.IsChecked();

	if (PreviewVC)
	{
		// Toggle the grid and worldbox.
		EditorOptions->bShowGrid = bShowGrid;
		EditorOptions->SaveConfig();
		PreviewVC->DrawHelper.bDrawGrid = bShowGrid;
		if (bShowGrid)
		{
			PreviewVC->ShowFlags |= SHOW_Grid;
		}
		else
		{
			PreviewVC->ShowFlags &= ~SHOW_Grid;
		}
	}
}

void WxCascade::OnViewAxes(wxCommandEvent& In)
{
	PreviewVC->bDrawOriginAxes = In.IsChecked();
}

void WxCascade::OnViewCounts(wxCommandEvent& In)
{
	PreviewVC->bDrawParticleCounts = In.IsChecked();
	EditorOptions->bShowParticleCounts = PreviewVC->bDrawParticleCounts;
	EditorOptions->SaveConfig();
}

void WxCascade::OnViewEvents(wxCommandEvent& In)
{
	PreviewVC->bDrawParticleEvents = In.IsChecked();
	EditorOptions->bShowParticleEvents = PreviewVC->bDrawParticleEvents;
	EditorOptions->SaveConfig();
}

void WxCascade::OnViewTimes(wxCommandEvent& In)
{
	PreviewVC->bDrawParticleTimes = In.IsChecked();
	EditorOptions->bShowParticleTimes = PreviewVC->bDrawParticleTimes;
	EditorOptions->SaveConfig();
}

void WxCascade::OnViewDistance(wxCommandEvent& In)
{
	PreviewVC->bDrawSystemDistance = In.IsChecked();
	EditorOptions->bShowParticleDistance = PreviewVC->bDrawSystemDistance;
	EditorOptions->SaveConfig();
}

void WxCascade::OnViewGeometry(wxCommandEvent& In)
{
	if (PreviewVC->FloorComponent)
	{
		PreviewVC->FloorComponent->HiddenEditor = !In.IsChecked();
		PreviewVC->FloorComponent->HiddenGame = PreviewVC->FloorComponent->HiddenEditor;

		EditorOptions->bShowFloor = !PreviewVC->FloorComponent->HiddenEditor;
		EditorOptions->SaveConfig();

		PreviewVC->PreviewScene.RemoveComponent(PreviewVC->FloorComponent);
		PreviewVC->PreviewScene.AddComponent(PreviewVC->FloorComponent, FMatrix::Identity);
	}
}

void WxCascade::OnViewGeometryProperties(wxCommandEvent& In)
{
	if (PreviewVC->FloorComponent)
	{
		WxPropertyWindowFrame* Properties = new WxPropertyWindowFrame;
		Properties->Create(this, -1, 0, &PropWindowNotifyHook);
		Properties->AllowClose();
		Properties->SetObject(PreviewVC->FloorComponent,0,1,1);
		Properties->SetTitle(*FString::Printf(TEXT("Properties: %s"), *PreviewVC->FloorComponent->GetPathName()));
		Properties->Show();
		PropWindowNotifyHook.Cascade = this;
		PropWindowNotifyHook.WindowOfInterest = (void*)(Properties->GetPropertyWindow());
	}
}

void WxCascade::OnLoopSimulation(wxCommandEvent& In)
{
	bResetOnFinish = In.IsChecked();

	if (!bResetOnFinish)
		bPendingReset = FALSE;
}

void WxCascade::OnShowPPBloom( wxCommandEvent& In )
{
	check(PreviewVC);
	if (In.IsChecked())
	{
		PreviewVC->ShowPPFlags |= CASC_SHOW_BLOOM;
	}
	else
	{
		PreviewVC->ShowPPFlags &= ~CASC_SHOW_BLOOM;
	}
	EditorOptions->ShowPPFlags = PreviewVC->ShowPPFlags;
	EditorOptions->SaveConfig();
	UpdatePostProcessChain();
}

void WxCascade::OnShowPPDOF( wxCommandEvent& In )
{
	check(PreviewVC);
	if (In.IsChecked())
	{
		PreviewVC->ShowPPFlags |= CASC_SHOW_DOF;
	}
	else
	{
		PreviewVC->ShowPPFlags &= ~CASC_SHOW_DOF;
	}
	EditorOptions->ShowPPFlags = PreviewVC->ShowPPFlags;
	EditorOptions->SaveConfig();
	UpdatePostProcessChain();
}

void WxCascade::OnShowPPMotionBlur( wxCommandEvent& In )
{
	check(PreviewVC);
	if (In.IsChecked())
	{
		PreviewVC->ShowPPFlags |= CASC_SHOW_MOTIONBLUR;
	}
	else
	{
		PreviewVC->ShowPPFlags &= ~CASC_SHOW_MOTIONBLUR;
	}
	EditorOptions->ShowPPFlags = PreviewVC->ShowPPFlags;
	EditorOptions->SaveConfig();
	UpdatePostProcessChain();
}

void WxCascade::OnShowPPVolumeMaterial( wxCommandEvent& In )
{
	check(PreviewVC);
	if (In.IsChecked())
	{
		PreviewVC->ShowPPFlags |= CASC_SHOW_PPVOLUME;
	}
	else
	{
		PreviewVC->ShowPPFlags &= ~CASC_SHOW_PPVOLUME;
	}
	EditorOptions->ShowPPFlags = PreviewVC->ShowPPFlags;
	EditorOptions->SaveConfig();
	UpdatePostProcessChain();
}

#if defined(_CASCADE_ENABLE_MODULE_DUMP_)
void WxCascade::OnViewModuleDump(wxCommandEvent& In)
{
	EmitterEdVC->bDrawModuleDump = !EmitterEdVC->bDrawModuleDump;
	EditorOptions->bShowModuleDump = EmitterEdVC->bDrawModuleDump;
	EditorOptions->SaveConfig();

	EmitterEdVC->Viewport->Invalidate();
}
#endif	//#if defined(_CASCADE_ENABLE_MODULE_DUMP_)

void WxCascade::OnPlay(wxCommandEvent& In)
{
	PreviewVC->TimeScale = SimSpeed;
}

void WxCascade::OnPause(wxCommandEvent& In)
{
	PreviewVC->TimeScale = 0.f;
}

void WxCascade::OnSpeed(wxCommandEvent& In)
{
	INT Id = In.GetId();

	FLOAT NewSimSpeed = 0.0f;
	INT SimID;

	switch (Id)
	{
	case IDM_CASCADE_SPEED_1:
		NewSimSpeed = 0.01f;
		SimID = IDM_CASCADE_SIM_1PERCENT;
		break;
	case IDM_CASCADE_SPEED_10:
		NewSimSpeed = 0.1f;
		SimID = IDM_CASCADE_SIM_10PERCENT;
		break;
	case IDM_CASCADE_SPEED_25:
		NewSimSpeed = 0.25f;
		SimID = IDM_CASCADE_SIM_25PERCENT;
		break;
	case IDM_CASCADE_SPEED_50:
		NewSimSpeed = 0.5f;
		SimID = IDM_CASCADE_SIM_50PERCENT;
		break;
	case IDM_CASCADE_SPEED_100:
		NewSimSpeed = 1.0f;
		SimID = IDM_CASCADE_SIM_NORMAL;
		break;
	}

	if (NewSimSpeed != 0.0f)
	{
		SimSpeed = NewSimSpeed;
		if (PreviewVC->TimeScale != 0.0f)
		{
			PreviewVC->TimeScale = SimSpeed;
		}
	}
}

void WxCascade::OnLoopSystem(wxCommandEvent& In)
{
	OnLoopSimulation(In);
}

void WxCascade::OnRealtime(wxCommandEvent& In)
{
	PreviewVC->SetRealtime(In.IsChecked());
}

void WxCascade::OnBackgroundColor(wxCommandEvent& In)
{
	wxColour wxColorIn(PreviewVC->BackgroundColor.R, PreviewVC->BackgroundColor.G, PreviewVC->BackgroundColor.B);

	wxColour wxColorOut = wxGetColourFromUser(this, wxColorIn);
	if (wxColorOut.Ok())
	{
		PreviewVC->BackgroundColor.R = wxColorOut.Red();
		PreviewVC->BackgroundColor.G = wxColorOut.Green();
		PreviewVC->BackgroundColor.B = wxColorOut.Blue();
	}

	EditorOptions->BackgroundColor = PreviewVC->BackgroundColor;
	EditorOptions->SaveConfig();
}

void WxCascade::OnToggleWireSphere(wxCommandEvent& In)
{
	PreviewVC->bDrawWireSphere = !PreviewVC->bDrawWireSphere;
	if (PreviewVC->bDrawWireSphere)
	{
		// display a dialog box asking fort the radius of the sphere
		WxDlgGenericStringEntry Dialog;
		INT Result = Dialog.ShowModal(TEXT("CascadeToggleWireSphere"), TEXT("SphereRadius"), *FString::Printf(TEXT("%f"), PreviewVC->WireSphereRadius));
		if (Result != wxID_OK)
		{
			// dialog was canceled
			PreviewVC->bDrawWireSphere = FALSE;
			ToolBar->ToggleTool(IDM_CASCADE_TOGGLE_WIRE_SPHERE, FALSE);
		}
		else
		{
			FLOAT NewRadius = appAtof(*Dialog.GetEnteredString());
			// if an invalid number was entered, cancel
			if (NewRadius < KINDA_SMALL_NUMBER)
			{
				PreviewVC->bDrawWireSphere = FALSE;
				ToolBar->ToggleTool(IDM_CASCADE_TOGGLE_WIRE_SPHERE, FALSE);
			}
			else
			{
				PreviewVC->WireSphereRadius = NewRadius;
			}
		}
	}
}

void WxCascade::OnUndo(wxCommandEvent& In)
{
	CascadeUndo();
}

void WxCascade::OnRedo(wxCommandEvent& In)
{
	CascadeRedo();
}

void WxCascade::OnPerformanceCheck(wxCommandEvent& In)
{
	debugf(TEXT("PERFORMANCE CHECK!"));
}

void WxCascade::OnLODLow(wxCommandEvent& In)
{
	if (!ToolBar || !PartSys || (PartSys->Emitters.Num() == 0))
	{
		return;
	}

	INT	Value = PartSys->Emitters(0)->LODLevels.Num() - 1;

	SetLODValue(Value);
	SetSelectedModule(SelectedEmitter, SelectedModule);
	EmitterEdVC->Viewport->Invalidate();
}

void WxCascade::OnLODLower(wxCommandEvent& In)
{
	if (!ToolBar || !PartSys || (PartSys->Emitters.Num() == 0))
	{
		return;
	}

	INT	LODValue = GetCurrentlySelectedLODLevelIndex();
	// Find the next lower LOD...
	// We can use any emitter, since they will all have the same number of LOD levels
	UParticleEmitter* Emitter	= PartSys->Emitters(0);
	if (Emitter)
	{
		for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
		{
			UParticleLODLevel* LODLevel	= Emitter->LODLevels(LODIndex);
			if (LODLevel)
			{
				if (LODLevel->Level > LODValue)
				{
					SetLODValue(LODLevel->Level);
					SetSelectedModule(SelectedEmitter, SelectedModule);
					EmitterEdVC->Viewport->Invalidate();
					break;
				}
			}
		}
	}
}

void WxCascade::OnLODAddBefore(wxCommandEvent& In)
{
	if (PartSys == NULL)
	{
		return;
	}

	// See if there is already a LOD level for this value...
	if (PartSys->Emitters.Num() > 0)
	{
		UParticleEmitter* FirstEmitter = PartSys->Emitters(0);
		if (FirstEmitter)
		{
			if (FirstEmitter->LODLevels.Num() >= 8)
			{
				appMsgf(AMT_OK, *(LocalizeUnrealEd("CascadeTooManyLODs")));
				return;
			}
		}

		INT CurrentLODIndex = GetCurrentlySelectedLODLevelIndex();
		if (CurrentLODIndex < 0)
		{
			return;
		}

		debugf(TEXT("Inserting LOD level at %d"), CurrentLODIndex);

		BeginTransaction(TEXT("CascadeLODAddBefore"));
		ModifyParticleSystem();

		for (INT EmitterIndex = 0; EmitterIndex < PartSys->Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter* Emitter = PartSys->Emitters(EmitterIndex);
			if (Emitter)
			{
				Emitter->CreateLODLevel(CurrentLODIndex);
			}
		}

		PartSys->LODDistances.InsertZeroed(CurrentLODIndex, 1);
		if (CurrentLODIndex == 0)
		{
			PartSys->LODDistances(CurrentLODIndex) = 0.0f;
		}
		else
		{
			PartSys->LODDistances(CurrentLODIndex) = PartSys->LODDistances(CurrentLODIndex - 1);
		}

		check(TransactionInProgress());
		EndTransaction(TEXT("CascadeLODAddBefore"));

		UpdateLODLevelControls();
		SetSelectedModule(SelectedEmitter, SelectedModule);
		CascadeTouch();

		wxCommandEvent DummyEvent;
		OnResetInLevel(DummyEvent);
	}
}

void WxCascade::OnLODAddAfter(wxCommandEvent& In)
{
	if (PartSys == NULL)
	{
		return;
	}

	// See if there is already a LOD level for this value...
	if (PartSys->Emitters.Num() > 0)
	{
		UParticleEmitter* FirstEmitter = PartSys->Emitters(0);
		if (FirstEmitter)
		{
			if (FirstEmitter->LODLevels.Num() >= 8)
			{
				appMsgf(AMT_OK, *(LocalizeUnrealEd("CascadeTooManyLODs")));
				return;
			}
		}

		INT CurrentLODIndex = GetCurrentlySelectedLODLevelIndex();
		CurrentLODIndex++;

		debugf(TEXT("Inserting LOD level at %d"), CurrentLODIndex);

		BeginTransaction(TEXT("CascadeLODAddAfter"));
		ModifyParticleSystem();

		for (INT EmitterIndex = 0; EmitterIndex < PartSys->Emitters.Num(); EmitterIndex++)
		{
			UParticleEmitter* Emitter = PartSys->Emitters(EmitterIndex);
			if (Emitter)
			{
				Emitter->CreateLODLevel(CurrentLODIndex);
			}
		}

		PartSys->LODDistances.InsertZeroed(CurrentLODIndex, 1);
		if (CurrentLODIndex == 0)
		{
			PartSys->LODDistances(CurrentLODIndex) = 0.0f;
		}
		else
		{
			PartSys->LODDistances(CurrentLODIndex) = PartSys->LODDistances(CurrentLODIndex - 1);
		}

		check(TransactionInProgress());
		EndTransaction(TEXT("CascadeLODAddAfter"));

		UpdateLODLevelControls();
		SetSelectedModule(SelectedEmitter, SelectedModule);
		CascadeTouch();

		wxCommandEvent DummyEvent;
		OnResetInLevel(DummyEvent);
	}
}

void WxCascade::OnLODHigher(wxCommandEvent& In)
{
	if (!ToolBar || !PartSys || (PartSys->Emitters.Num() == 0))
	{
		return;
	}

	INT	LODValue = GetCurrentlySelectedLODLevelIndex();

	// Find the next higher LOD...
	// We can use any emitter, since they will all have the same number of LOD levels
	UParticleEmitter* Emitter	= PartSys->Emitters(0);
	if (Emitter)
	{
		// Go from the low to the high...
		for (INT LODIndex = Emitter->LODLevels.Num() - 1; LODIndex >= 0; LODIndex--)
		{
			UParticleLODLevel* LODLevel	= Emitter->LODLevels(LODIndex);
			if (LODLevel)
			{
				if (LODLevel->Level < LODValue)
				{
					SetLODValue(LODLevel->Level);
					SetSelectedModule(SelectedEmitter, SelectedModule);
					EmitterEdVC->Viewport->Invalidate();
					break;
				}
			}
		}
	}
}

void WxCascade::OnLODHigh(wxCommandEvent& In)
{
	if (!ToolBar || !PartSys || (PartSys->Emitters.Num() == 0))
	{
		return;
	}

	INT	Value = 0;

	SetLODValue(Value);
	SetSelectedModule(SelectedEmitter, SelectedModule);
	EmitterEdVC->Viewport->Invalidate();
}

void WxCascade::OnLODDelete(wxCommandEvent& In)
{
	if (!ToolBar || !PartSys)
	{
		return;
	}

	UParticleEmitter* Emitter = PartSys->Emitters(0);
	if (Emitter == NULL)
	{
		return;
	}

	INT	Selection = GetCurrentlySelectedLODLevelIndex();
	if ((Selection < 0) || ((Selection == 0) && (Emitter->LODLevels.Num() == 1)))
	{
		appMsgf(AMT_OK, *(LocalizeUnrealEd("CascadeCantDeleteLOD")));
		return;
	}

	// Delete the setting...
	BeginTransaction(TEXT("CascadeDeleteLOD"));
	ModifyParticleSystem();

	// Remove the LOD entry from the distance array
	for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel	= Emitter->LODLevels(LODIndex);
		if (LODLevel)
		{
			if ((LODLevel->Level == Selection) && (PartSys->LODDistances.Num() > LODLevel->Level))
			{
				PartSys->LODDistances.Remove(LODLevel->Level);
				break;
			}
		}
	}

	// Remove the level from each emitter in the system
	for (INT EmitterIndex = 0; EmitterIndex < PartSys->Emitters.Num(); EmitterIndex++)
	{
		Emitter = PartSys->Emitters(EmitterIndex);
		if (Emitter)
		{
			for (INT LODIndex = 0; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
			{
				UParticleLODLevel* LODLevel	= Emitter->LODLevels(LODIndex);
				if (LODLevel)
				{
					if (LODLevel->Level == Selection)
					{
						// Clear out the flags from the modules.
						LODLevel->RequiredModule->LODValidity &= ~(1 << LODLevel->Level);
						LODLevel->SpawnModule->LODValidity &= ~(1 << LODLevel->Level);
						if (LODLevel->TypeDataModule)
						{
							LODLevel->TypeDataModule->LODValidity &= ~(1 << LODLevel->Level);
						}

						for (INT ModuleIndex = 0; ModuleIndex < LODLevel->Modules.Num(); ModuleIndex++)
						{
							UParticleModule* PModule = LODLevel->Modules(ModuleIndex);
							if (PModule)
							{
								PModule->LODValidity  &= ~(1 << LODLevel->Level);
							}
						}

						// Delete it and shift all down
						Emitter->LODLevels.Remove(LODIndex);

						for (; LODIndex < Emitter->LODLevels.Num(); LODIndex++)
						{
							UParticleLODLevel* RemapLODLevel	= Emitter->LODLevels(LODIndex);
							if (RemapLODLevel)
							{
								RemapLODLevel->SetLevelIndex(RemapLODLevel->Level - 1);
							}
						}
						break;
					}
				}
			}
		}
	}

	check(TransactionInProgress());
	EndTransaction(TEXT("CascadeDeleteLOD"));

	CascadeTouch();
	PropertyWindow->Rebuild();

	wxCommandEvent DummyEvent;
	OnResetInLevel(DummyEvent);
}

///////////////////////////////////////////////////////////////////////////////////////
// Properties window NotifyHook stuff

void WxCascade::NotifyDestroy( void* Src )
{

}

void WxCascade::NotifyPreChange( void* Src, UProperty* PropertyAboutToChange )
{

}

void WxCascade::NotifyPostChange( void* Src, UProperty* PropertyThatChanged )
{
	if (SelectedModule)
	{
		SelectedModule->PostEditChange(PropertyThatChanged);
	}
	else
	if (SelectedEmitter)
	{
		SelectedEmitter->PostEditChange(PropertyThatChanged);
	}
	else
	if (PartSys)
	{
		PartSys->PostEditChange(PropertyThatChanged);
	}
}

void WxCascade::NotifyPreChange( void* Src, FEditPropertyChain* PropertyChain )
{
	BeginTransaction(TEXT("CascadePropertyChange"));
	ModifyParticleSystem();

	CurveToReplace = NULL;

	// get the property that is being edited
	UObjectProperty* ObjProp = Cast<UObjectProperty>(PropertyChain->GetActiveNode()->GetValue());
	if (ObjProp && 
		(ObjProp->PropertyClass->IsChildOf(UDistributionFloat::StaticClass()) || 
		 ObjProp->PropertyClass->IsChildOf(UDistributionVector::StaticClass()))
		 )
	{
		UParticleModuleParameterDynamic* DynParamModule = Cast<UParticleModuleParameterDynamic>(SelectedModule);
		if (DynParamModule)
		{
			// Grab the curves...
			DynParamModule->GetCurveObjects(DynParamCurves);
		}
		else
		{
			UObject* EditedObject = NULL;
			if (SelectedModule)
			{
				EditedObject = SelectedModule;
			}
			else
	//		if (SelectedEmitter)
			{
				EditedObject = SelectedEmitter;
			}

			// calculate offset from object to property being edited
			DWORD Offset = 0;
			UObject* BaseObject = EditedObject;
			for (FEditPropertyChain::TIterator It(PropertyChain->GetHead()); It; ++It )
			{
				Offset += It->Offset;

				// don't go past the active property
				if (*It == ObjProp)
				{
					break;
				}

				// If it is an object property, then reset our base pointer/offset
				if (It->IsA(UObjectProperty::StaticClass()))
				{
					BaseObject = *(UObject**)((BYTE*)BaseObject + Offset);
					Offset = 0;
				}
			}

			BYTE* CurvePointer = (BYTE*)BaseObject + Offset;
			UObject** ObjPtrPtr = (UObject**)CurvePointer;
			UObject* ObjPtr = *(ObjPtrPtr);
			CurveToReplace = ObjPtr;

			check(CurveToReplace); // These properties are 'noclear', so should always have a curve here!
		}
	}

	if (SelectedModule)
	{
		if (PropertyChain->GetActiveNode()->GetValue()->GetName() == TEXT("InterpolationMethod"))
		{
			UParticleModuleRequired* ReqMod = Cast<UParticleModuleRequired>(SelectedModule);
			if (ReqMod)
			{
				PreviousInterpolationMethod = (EParticleSubUVInterpMethod)(ReqMod->InterpolationMethod);
			}
		}
	}
}

void WxCascade::NotifyPostChange( void* Src, FEditPropertyChain* PropertyChain )
{
	UParticleModuleParameterDynamic* DynParamModule = Cast<UParticleModuleParameterDynamic>(SelectedModule);
	if (DynParamModule)
	{
		if (DynParamCurves.Num() > 0)
		{
			// Grab the curves...
			TArray<FParticleCurvePair> DPCurves;
			DynParamModule->GetCurveObjects(DPCurves);

			check(DPCurves.Num() == DynParamCurves.Num());
			for (INT CurveIndex = 0; CurveIndex < DynParamCurves.Num(); CurveIndex++)
			{
				UObject* OldCurve = DynParamCurves(CurveIndex).CurveObject;
				UObject* NewCurve = DPCurves(CurveIndex).CurveObject;
				if (OldCurve != NewCurve)
				{
					PartSys->CurveEdSetup->ReplaceCurve(OldCurve, NewCurve);
					CurveEd->CurveChanged();
				}
			}
			DynParamCurves.Empty();
		}
	}

	if (CurveToReplace)
	{
		// This should be the same property we just got in NotifyPreChange!
		UObjectProperty* ObjProp = Cast<UObjectProperty>(PropertyChain->GetActiveNode()->GetValue());
		check(ObjProp);
		check(ObjProp->PropertyClass->IsChildOf(UDistributionFloat::StaticClass()) || ObjProp->PropertyClass->IsChildOf(UDistributionVector::StaticClass()));

		UObject* EditedObject = NULL;
		if (SelectedModule)
		{
			EditedObject = SelectedModule;
		}
		else
//		if (SelectedEmitter)
		{
			EditedObject = SelectedEmitter;
		}

		// calculate offset from object to property being edited
		DWORD Offset = 0;
		UObject* BaseObject = EditedObject;
		for (FEditPropertyChain::TIterator It(PropertyChain->GetHead()); It; ++It )
		{
			Offset += It->Offset;

			// don't go past the active property
			if (*It == ObjProp)
			{
				break;
			}

			// If it is an object property, then reset our base pointer/offset
			if (It->IsA(UObjectProperty::StaticClass()))
			{
				BaseObject = *(UObject**)((BYTE*)BaseObject + Offset);
				Offset = 0;
			}
		}

		BYTE* CurvePointer = (BYTE*)BaseObject + Offset;
		UObject** ObjPtrPtr = (UObject**)CurvePointer;
		UObject* NewCurve = *(ObjPtrPtr);
		
		if (NewCurve)
		{
			PartSys->CurveEdSetup->ReplaceCurve(CurveToReplace, NewCurve);
			CurveEd->CurveChanged();
		}
	}

	if (SelectedModule || SelectedEmitter)
	{
		if (PropertyChain->GetActiveNode()->GetValue()->GetName() == TEXT("InterpolationMethod"))
		{
			UParticleModuleRequired* ReqMod = Cast<UParticleModuleRequired>(SelectedModule);
			if (ReqMod && SelectedEmitter)
			{
				if (ReqMod->InterpolationMethod != PreviousInterpolationMethod)
				{
					INT CurrentLODLevel = GetCurrentlySelectedLODLevelIndex();
					if (CurrentLODLevel == 0)
					{
						// The main on is being changed...
						// Check all other LOD levels...
						for (INT LODIndex = 1; LODIndex < SelectedEmitter->LODLevels.Num(); LODIndex++)
						{
							UParticleLODLevel* CheckLOD = SelectedEmitter->LODLevels(LODIndex);
							if (CheckLOD)
							{
								UParticleModuleRequired* CheckReq = CheckLOD->RequiredModule;
								if (CheckReq)
								{
									if (ReqMod->InterpolationMethod == PSUVIM_None)
									{
										CheckReq->InterpolationMethod = PSUVIM_None;
									}
									else
									{
										if (CheckReq->InterpolationMethod == PSUVIM_None)
										{
											CheckReq->InterpolationMethod = ReqMod->InterpolationMethod;
										}
									}
								}
							}
						}
					}
					else
					{
						// The main on is being changed...
						// Check all other LOD levels...
						UParticleLODLevel* CheckLOD = SelectedEmitter->LODLevels(0);
						if (CheckLOD)
						{
							UBOOL bWarn = FALSE;
							UParticleModuleRequired* CheckReq = CheckLOD->RequiredModule;
							if (CheckReq)
							{
								if (ReqMod->InterpolationMethod == PSUVIM_None)
								{
									if (CheckReq->InterpolationMethod != PSUVIM_None)
									{
										ReqMod->InterpolationMethod = PreviousInterpolationMethod;
										bWarn = TRUE;
									}
								}
								else
								{
									if (CheckReq->InterpolationMethod == PSUVIM_None)
									{
										ReqMod->InterpolationMethod = PreviousInterpolationMethod;
										bWarn = TRUE;
									}
								}
							}

							if (bWarn == TRUE)
							{
								appMsgf(AMT_OK, *(LocalizeUnrealEd("Cascade_InterpolationMethodLODWarning")));
								PropertyWindow->Rebuild();
							}
						}
					}
				}
			}
		}

		PartSys->PostEditChange(PropertyChain->GetActiveNode()->GetValue());

		if (SelectedModule)
		{
			if (SelectedModule->IsDisplayedInCurveEd(CurveEd->EdSetup))
			{
				TArray<FParticleCurvePair> Curves;
				SelectedModule->GetCurveObjects(Curves);

				for (INT i=0; i<Curves.Num(); i++)
				{
					CurveEd->EdSetup->ChangeCurveColor(Curves(i).CurveObject, SelectedModule->ModuleEditorColor);
				}
			}
		}
	}

	PartSys->ThumbnailImageOutOfDate = TRUE;

	check(TransactionInProgress());
	EndTransaction(TEXT("CascadePropertyChange"));

	CurveEd->CurveChanged();
	EmitterEdVC->Viewport->Invalidate();
}

void WxCascade::NotifyExec( void* Src, const TCHAR* Cmd )
{
	GUnrealEd->NotifyExec(Src, Cmd);
}

///////////////////////////////////////////////////////////////////////////////////////
// Utils
void WxCascade::CreateNewModule(INT ModClassIndex)
{
	if (SelectedEmitter == NULL)
	{
		return;
	}

	INT CurrLODLevel = GetCurrentlySelectedLODLevelIndex();
	if (CurrLODLevel != 0)
	{
		// Don't allow creating modules if not at highest LOD
		appMsgf(AMT_OK, *(LocalizeUnrealEd(TEXT("CascadeLODAddError"))));
		return;
	}

	UClass* NewModClass = ParticleModuleClasses(ModClassIndex);
	check(NewModClass->IsChildOf(UParticleModule::StaticClass()));

	UBOOL bIsEventGenerator = FALSE;

	if (NewModClass->IsChildOf(UParticleModuleTypeDataBase::StaticClass()))
	{
		// Make sure there isn't already a TypeData module applied!
		UParticleLODLevel* LODLevel = SelectedEmitter->GetLODLevel(0);
		if (LODLevel->TypeDataModule != 0)
		{
			appMsgf(AMT_OK, *LocalizeUnrealEd("Error_TypeDataModuleAlreadyPresent"));
			return;
		}
	}
	else
	if (NewModClass == UParticleModuleEventGenerator::StaticClass())
	{
		bIsEventGenerator = TRUE;
		// Make sure there isn't already an EventGenerator module applied!
		UParticleLODLevel* LODLevel = SelectedEmitter->GetLODLevel(0);
		if (LODLevel->EventGenerator != NULL)
		{
			appMsgf(AMT_OK, *LocalizeUnrealEd("Error_EventGeneratorModuleAlreadyPresent"));
			return;
		}
	}
	else
	if (NewModClass == UParticleModuleParameterDynamic::StaticClass())
	{
		// Make sure there isn't already an DynamicParameter module applied!
		UParticleLODLevel* LODLevel = SelectedEmitter->GetLODLevel(0);
		for (INT CheckMod = 0; CheckMod < LODLevel->Modules.Num(); CheckMod++)
		{
			UParticleModuleParameterDynamic* DynamicParamMod = Cast<UParticleModuleParameterDynamic>(LODLevel->Modules(CheckMod));
			if (DynamicParamMod)
			{
				appMsgf(AMT_OK, *LocalizeUnrealEd("Error_DynamicParameterModuleAlreadyPresent"));
				return;
			}
		}
	}

	BeginTransaction(TEXT("CreateNewModule"));
	ModifyParticleSystem();
	ModifySelectedObjects();

	PartSys->PreEditChange(NULL);
	PartSysComp->PreEditChange(NULL);

	// Construct it and add to selected emitter.
	UParticleModule* NewModule = ConstructObject<UParticleModule>(NewModClass, PartSys, NAME_None, RF_Transactional);
	NewModule->ModuleEditorColor = FColor::MakeRandomColor();
	NewModule->SetToSensibleDefaults(SelectedEmitter);
	NewModule->LODValidity = 1;

	UParticleLODLevel* LODLevel	= SelectedEmitter->GetLODLevel(0);
	if (bIsEventGenerator == TRUE)
	{
		LODLevel->Modules.InsertItem(NewModule, 0);
		LODLevel->EventGenerator = Cast<UParticleModuleEventGenerator>(NewModule);
	}
	else
	{
		LODLevel->Modules.AddItem(NewModule);
	}

	for (INT LODIndex = 1; LODIndex < SelectedEmitter->LODLevels.Num(); LODIndex++)
	{
		LODLevel = SelectedEmitter->GetLODLevel(LODIndex);
		NewModule->LODValidity |= (1 << LODIndex);
		if (bIsEventGenerator == TRUE)
		{
			LODLevel->Modules.InsertItem(NewModule, 0);
			LODLevel->EventGenerator = Cast<UParticleModuleEventGenerator>(NewModule);
		}
		else
		{
			LODLevel->Modules.AddItem(NewModule);
		}
	}

	SelectedEmitter->UpdateModuleLists();

	PartSysComp->PostEditChange(NULL);
	PartSys->PostEditChange(NULL);

	EndTransaction(TEXT("CreateNewModule"));

	PartSys->MarkPackageDirty();

	// Refresh viewport
	EmitterEdVC->Viewport->Invalidate();
}

void WxCascade::PasteCurrentModule()
{
	if (!SelectedEmitter)
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("Error_MustSelectEmitter"));
		return;
	}

	INT CurrLODIndex = GetCurrentlySelectedLODLevelIndex();
	if (CurrLODIndex != 0)
	{
		// Don't allow pasting modules if not at highest LOD
		appMsgf(AMT_OK, *(LocalizeUnrealEd(TEXT("CascadeLODPasteError"))));
		return;
	}

	check(CopyModule);

	UObject* pkDupObject = 
		GEditor->StaticDuplicateObject(CopyModule, CopyModule, PartSys, TEXT("None"));
	if (pkDupObject)
	{
		UParticleModule* Module	= Cast<UParticleModule>(pkDupObject);
		Module->ModuleEditorColor = FColor::MakeRandomColor();
		UParticleLODLevel* LODLevel	= SelectedEmitter->GetLODLevel(0);
		InsertModule(Module, SelectedEmitter, LODLevel->Modules.Num());
	}
}

void WxCascade::CopyModuleToEmitter(UParticleModule* pkSourceModule, UParticleEmitter* pkTargetEmitter, UParticleSystem* pkTargetSystem)
{
    check(pkSourceModule);
    check(pkTargetEmitter);
	check(pkTargetSystem);

	INT CurrLODIndex = GetCurrentlySelectedLODLevelIndex();
	if (CurrLODIndex != 0)
	{
		// Don't allow copying modules if not at highest LOD
		appMsgf(AMT_OK, *(LocalizeUnrealEd(TEXT("CascadeLODCopyError"))));
		return;
	}

	UObject* DupObject = GEditor->StaticDuplicateObject(pkSourceModule, pkSourceModule, pkTargetSystem, TEXT("None"));
	if (DupObject)
	{
		UParticleModule* Module	= Cast<UParticleModule>(DupObject);
		Module->ModuleEditorColor = FColor::MakeRandomColor();

		UParticleLODLevel* LODLevel;

		if (EmitterEdVC->DraggedModule == pkSourceModule)
		{
			EmitterEdVC->DraggedModules(0) = Module;
			// If we are dragging, we need to copy all the LOD modules
			for (INT LODIndex = 1; LODIndex < pkTargetEmitter->LODLevels.Num(); LODIndex++)
			{
				LODLevel	= pkTargetEmitter->GetLODLevel(LODIndex);

				UParticleModule* CopySource = EmitterEdVC->DraggedModules(LODIndex);
				if (CopySource)
				{
					DupObject = GEditor->StaticDuplicateObject(CopySource, CopySource, pkTargetSystem, TEXT("None"));
					if (DupObject)
					{
						UParticleModule* NewModule	= Cast<UParticleModule>(DupObject);
						NewModule->ModuleEditorColor = Module->ModuleEditorColor;
						EmitterEdVC->DraggedModules(LODIndex) = NewModule;
					}
				}
				else
				{
					warnf(TEXT("Missing dragged module!"));
				}
			}
		}

		LODLevel	= pkTargetEmitter->GetLODLevel(0);
		InsertModule(Module, pkTargetEmitter, LODLevel->Modules.Num(), FALSE);
	}
}

void WxCascade::SetSelectedEmitter( UParticleEmitter* NewSelectedEmitter )
{
	SetSelectedModule(NewSelectedEmitter, NULL);
}

void WxCascade::SetSelectedModule( UParticleEmitter* NewSelectedEmitter, UParticleModule* NewSelectedModule )
{
	SelectedEmitter = NewSelectedEmitter;

	INT CurrLODIndex = GetCurrentlySelectedLODLevelIndex();
	if (CurrLODIndex < 0)
	{
		return;
	}

	UParticleLODLevel* LODLevel = NULL;
	// Make sure it's the correct LOD level...
	if (SelectedEmitter)
	{
		LODLevel = SelectedEmitter->GetLODLevel(CurrLODIndex);
		if (NewSelectedModule)
		{
			INT	ModuleIndex	= INDEX_NONE;
			for (INT LODLevelCheck = 0; LODLevelCheck < SelectedEmitter->LODLevels.Num(); LODLevelCheck++)
			{
				UParticleLODLevel* CheckLODLevel	= SelectedEmitter->LODLevels(LODLevelCheck);
				if (LODLevel)
				{
					// Check the type data...
					if (CheckLODLevel->TypeDataModule &&
						(CheckLODLevel->TypeDataModule == NewSelectedModule))
					{
						ModuleIndex = INDEX_TYPEDATAMODULE;
					}

					// Check the required module...
					if (ModuleIndex == INDEX_NONE)
					{
						if (CheckLODLevel->RequiredModule == NewSelectedModule)
						{
							ModuleIndex = INDEX_REQUIREDMODULE;
						}
					}

					// Check the spawn...
					if (ModuleIndex == INDEX_NONE)
					{
						if (CheckLODLevel->SpawnModule == NewSelectedModule)
						{
							ModuleIndex = INDEX_SPAWNMODULE;
						}
					}

					// Check the rest...
					if (ModuleIndex == INDEX_NONE)
					{
						for (INT ModuleCheck = 0; ModuleCheck < CheckLODLevel->Modules.Num(); ModuleCheck++)
						{
							if (CheckLODLevel->Modules(ModuleCheck) == NewSelectedModule)
							{
								ModuleIndex = ModuleCheck;
								break;
							}
						}
					}
				}

				if (ModuleIndex != INDEX_NONE)
				{
					break;
				}
			}

			switch (ModuleIndex)
			{
			case INDEX_NONE:
				break;
			case INDEX_TYPEDATAMODULE:
				NewSelectedModule = LODLevel->TypeDataModule;
				break;
			case INDEX_REQUIREDMODULE:
				NewSelectedModule = LODLevel->RequiredModule;
				break;
			case INDEX_SPAWNMODULE:
				NewSelectedModule = LODLevel->SpawnModule;
				break;
			default:
				NewSelectedModule = LODLevel->Modules(ModuleIndex);
				break;
			}
			SelectedModuleIndex	= ModuleIndex;
		}
	}

	SelectedModule = NewSelectedModule;

	UBOOL bReadOnly = FALSE;
	UObject* PropObj = PartSys;
	if (SelectedEmitter)
	{
		if (SelectedModule)
		{
			if (LODLevel != NULL)
			{
				if (LODLevel->Level != CurrLODIndex)
				{
					bReadOnly = TRUE;
				}
				else
				{
					bReadOnly = !LODLevel->IsModuleEditable(SelectedModule);
				}
			}
			PropObj = SelectedModule;
		}
		else
		{
			// Only allowing editing the SelectedEmitter 
			// properties when at the highest LOD level.
			if (!(LODLevel && (LODLevel->Level == 0)))
			{
				bReadOnly = TRUE;
			}
			PropObj = SelectedEmitter;
		}
	}
	PropertyWindow->SetReadOnly(bReadOnly);
	PropertyWindow->SetObject(PropObj, 1, 0, 1);

	SetSelectedInCurveEditor();
	EmitterEdVC->Viewport->Invalidate();
}

// Insert the selected module into the target emitter at the desired location.
UBOOL WxCascade::InsertModule(UParticleModule* Module, UParticleEmitter* TargetEmitter, INT TargetIndex, UBOOL bSetSelected)
{
	if (!Module || !TargetEmitter || TargetIndex == INDEX_NONE)
	{
		return FALSE;
	}

	INT CurrLODIndex = GetCurrentlySelectedLODLevelIndex();
	if (CurrLODIndex != 0)
	{
		// Don't allow moving modules if not at highest LOD
		warnf(*(LocalizeUnrealEd(TEXT("CascadeLODMoveError"))));
		return FALSE;
	}

	// Cannot insert the same module more than once into the same emitter.
	UParticleLODLevel* LODLevel	= TargetEmitter->GetLODLevel(0);
	for(INT i = 0; i < LODLevel->Modules.Num(); i++)
	{
		if (LODLevel->Modules(i) == Module)
		{
			appMsgf(AMT_OK, *LocalizeUnrealEd("Error_ModuleCanOnlyBeUsedInEmitterOnce"));
			return FALSE;
		}
	}

	if (Module->IsA(UParticleModuleParameterDynamic::StaticClass()))
	{
		// Make sure there isn't already an DynamicParameter module applied!
		UParticleLODLevel* LODLevel = TargetEmitter->GetLODLevel(0);
		for (INT CheckMod = 0; CheckMod < LODLevel->Modules.Num(); CheckMod++)
		{
			UParticleModuleParameterDynamic* DynamicParamMod = Cast<UParticleModuleParameterDynamic>(LODLevel->Modules(CheckMod));
			if (DynamicParamMod)
			{
				appMsgf(AMT_OK, *LocalizeUnrealEd("Error_DynamicParameterModuleAlreadyPresent"));
				return FALSE;
			}
		}
	}

	// If the Spawn or Required modules are being 're-inserted', do nothing!
	if ((LODLevel->SpawnModule == Module) ||
		(LODLevel->RequiredModule == Module))
	{
		return FALSE;
	}

	BeginTransaction(TEXT("InsertModule"));
	ModifyEmitter(TargetEmitter);
	ModifyParticleSystem();

	// Insert in desired location in new Emitter
	PartSys->PreEditChange(NULL);

	if (Module->IsA(UParticleModuleTypeDataBase::StaticClass()))
	{
		if (LODLevel->TypeDataModule)
		{
			LODLevel->TypeDataModule = Module;

			if (EmitterEdVC->DraggedModules.Num() > 0)
			{
				// Swap the modules in all the LOD levels
				for (INT LODIndex = 1; LODIndex < TargetEmitter->LODLevels.Num(); LODIndex++)
				{
					UParticleLODLevel*	LODLevel	= TargetEmitter->GetLODLevel(LODIndex);
					UParticleModule*	Module		= EmitterEdVC->DraggedModules(LODIndex);

					LODLevel->TypeDataModule	= Module;
				}
			}
		}
	}
	else if (Module->IsA(UParticleModuleSpawn::StaticClass()))
	{
		// There can be only one...
		LODLevel->SpawnModule = CastChecked<UParticleModuleSpawn>(Module);
		if (EmitterEdVC->DraggedModules.Num() > 0)
		{
			// Swap the modules in all the LOD levels
			for (INT LODIndex = 1; LODIndex < TargetEmitter->LODLevels.Num(); LODIndex++)
			{
				UParticleLODLevel* LODLevel	= TargetEmitter->GetLODLevel(LODIndex);
				UParticleModuleSpawn* Module = CastChecked<UParticleModuleSpawn>(EmitterEdVC->DraggedModules(LODIndex));
				LODLevel->SpawnModule = Module;
			}
		}
	}
	else if (Module->IsA(UParticleModuleRequired::StaticClass()))
	{
		// There can be only one...
		LODLevel->RequiredModule = CastChecked<UParticleModuleRequired>(Module);
		if (EmitterEdVC->DraggedModules.Num() > 0)
		{
			// Swap the modules in all the LOD levels
			for (INT LODIndex = 1; LODIndex < TargetEmitter->LODLevels.Num(); LODIndex++)
			{
				UParticleLODLevel* LODLevel	= TargetEmitter->GetLODLevel(LODIndex);
				UParticleModuleRequired* Module = CastChecked<UParticleModuleRequired>(EmitterEdVC->DraggedModules(LODIndex));
				LODLevel->RequiredModule = Module;
			}
		}
	}
	else
	{
		INT NewModuleIndex = Clamp<INT>(TargetIndex, 0, LODLevel->Modules.Num());
		LODLevel->Modules.Insert(NewModuleIndex);
		LODLevel->Modules(NewModuleIndex) = Module;

		if (EmitterEdVC->DraggedModules.Num() > 0)
		{
			// Swap the modules in all the LOD levels
			for (INT LODIndex = 1; LODIndex < TargetEmitter->LODLevels.Num(); LODIndex++)
			{
				UParticleLODLevel*	LODLevel	= TargetEmitter->GetLODLevel(LODIndex);
				UParticleModule*	Module		= EmitterEdVC->DraggedModules(LODIndex);

				LODLevel->Modules.Insert(NewModuleIndex);
				LODLevel->Modules(NewModuleIndex)	= Module;
			}
		}
	}
	EmitterEdVC->DraggedModules.Empty();

	TargetEmitter->UpdateModuleLists();

	PartSys->PostEditChange(NULL);

	// Update selection
    if (bSetSelected)
    {
        SetSelectedModule(TargetEmitter, Module);
    }

	EndTransaction(TEXT("InsertModule"));

	PartSys->MarkPackageDirty();
	EmitterEdVC->Viewport->Invalidate();

	return TRUE;
}

// Delete entire Emitter from System
// Garbage collection will clear up any unused modules.
void WxCascade::DeleteSelectedEmitter()
{
	if (!SelectedEmitter)
		return;

	check(PartSys->Emitters.ContainsItem(SelectedEmitter));

	INT	CurrLODSetting	= GetCurrentlySelectedLODLevelIndex();
	if (SelectedEmitter->IsLODLevelValid(CurrLODSetting) == FALSE)
	{
		return;
	}

	// If there are differences in the enabled states of the LOD levels for an emitter,
	// prompt the user to ensure they want to delete it...
	UParticleLODLevel* LODLevel = SelectedEmitter->LODLevels(0);
	UBOOL bEnabledStateDifferent = FALSE;
	UBOOL bEnabled = LODLevel->bEnabled;
	for (INT LODIndex = 1; (LODIndex < SelectedEmitter->LODLevels.Num()) && !bEnabledStateDifferent; LODIndex++)
	{
		LODLevel = SelectedEmitter->LODLevels(LODIndex);
		if (bEnabled != LODLevel->bEnabled)
		{
			bEnabledStateDifferent = TRUE;
		}
		else
		{
			if (LODLevel->IsModuleEditable(LODLevel->RequiredModule))
			{
				bEnabledStateDifferent = TRUE;
			}
			if (LODLevel->IsModuleEditable(LODLevel->SpawnModule))
			{
				bEnabledStateDifferent = TRUE;
			}
			if (LODLevel->TypeDataModule && LODLevel->IsModuleEditable(LODLevel->TypeDataModule))
			{
				bEnabledStateDifferent = TRUE;
			}

			for (INT CheckModIndex = 0; CheckModIndex < LODLevel->Modules.Num(); CheckModIndex++)
			{
				if (LODLevel->IsModuleEditable(LODLevel->Modules(CheckModIndex)))
				{
					bEnabledStateDifferent = TRUE;
				}
			}
		}
	}

	if (bEnabledStateDifferent == TRUE)
	{
		if (appMsgf(AMT_YesNo, *LocalizeUnrealEd("EmitterDeleteConfirm")) == FALSE)
		{
			return;
		}
	}

	BeginTransaction(TEXT("DeleteSelectedEmitter"));
	ModifyParticleSystem();

	PartSys->PreEditChange(NULL);

	SelectedEmitter->RemoveEmitterCurvesFromEditor(CurveEd->EdSetup);
	CurveEd->CurveChanged();

	PartSys->Emitters.RemoveItem(SelectedEmitter);

	PartSys->PostEditChange(NULL);

	SetSelectedEmitter(NULL);

	EndTransaction(TEXT("DeleteSelectedEmitter"));

	PartSys->MarkPackageDirty();
	EmitterEdVC->Viewport->Invalidate();
}

// Move the selected amitter by MoveAmount in the array of Emitters.
void WxCascade::MoveSelectedEmitter(INT MoveAmount)
{
	if (!SelectedEmitter)
		return;

	BeginTransaction(TEXT("MoveSelectedEmitter"));
	ModifyParticleSystem();

	INT CurrentEmitterIndex = PartSys->Emitters.FindItemIndex(SelectedEmitter);
	check(CurrentEmitterIndex != INDEX_NONE);

	INT NewEmitterIndex = Clamp<INT>(CurrentEmitterIndex + MoveAmount, 0, PartSys->Emitters.Num() - 1);

	if (NewEmitterIndex != CurrentEmitterIndex)
	{
		PartSys->PreEditChange(NULL);

		PartSys->Emitters.RemoveItem(SelectedEmitter);
		PartSys->Emitters.InsertZeroed(NewEmitterIndex);
		PartSys->Emitters(NewEmitterIndex) = SelectedEmitter;

		PartSys->PostEditChange(NULL);

		EmitterEdVC->Viewport->Invalidate();
	}

	EndTransaction(TEXT("MoveSelectedEmitter"));

	PartSys->MarkPackageDirty();
}

// Export the selected emitter for importing into another particle system
void WxCascade::ExportSelectedEmitter()
{
	if (!SelectedEmitter)
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd("Error_NoEmitterSelectedForExport"));
		return;
	}

	for ( USelection::TObjectIterator Itor = GEditor->GetSelectedObjects()->ObjectItor() ; Itor ; ++Itor )
	{
		UParticleSystem* DestPartSys = Cast<UParticleSystem>(*Itor);
		if (DestPartSys && (DestPartSys != PartSys))
		{
			INT NewCount = 0;
			if (DestPartSys->Emitters.Num() > 0)
			{
				UParticleEmitter* DestEmitter0 = DestPartSys->Emitters(0);

				NewCount = DestEmitter0->LODLevels.Num() - SelectedEmitter->LODLevels.Num();
				if (NewCount > 0)
				{
					// There are more LODs in the destination than the source... Add enough to cover.
					INT StartIndex = SelectedEmitter->LODLevels.Num();
					for (INT InsertIndex = 0; InsertIndex < NewCount; InsertIndex++)
					{
						SelectedEmitter->CreateLODLevel(StartIndex + InsertIndex, TRUE);
					}
					SelectedEmitter->UpdateModuleLists();
				}
				else
				if (NewCount < 0)
				{
					INT InsertCount = -NewCount;
					// There are fewer LODs in the destination than the source... Add enough to cover.
					INT StartIndex = DestEmitter0->LODLevels.Num();
					for (INT EmitterIndex = 0; EmitterIndex < DestPartSys->Emitters.Num(); EmitterIndex++)
					{
						UParticleEmitter* DestEmitter = DestPartSys->Emitters(EmitterIndex);
						if (DestEmitter)
						{
							for (INT InsertIndex = 0; InsertIndex < InsertCount; InsertIndex++)
							{
								DestEmitter->CreateLODLevel(StartIndex + InsertIndex, FALSE);
							}
							DestEmitter->UpdateModuleLists();
						}
					}

					// Add the slots in the LODDistances array
					DestPartSys->LODDistances.AddZeroed(InsertCount);
					for (INT DistIndex = StartIndex; DistIndex < DestPartSys->LODDistances.Num(); DistIndex++)
					{
						DestPartSys->LODDistances(DistIndex) = DistIndex * 2500.0f;
					}
				}
			}
			else
			{
				INT InsertCount = SelectedEmitter->LODLevels.Num();
				// Add the slots in the LODDistances array
				DestPartSys->LODDistances.AddZeroed(InsertCount);
				for (INT DistIndex = 0; DistIndex < InsertCount; DistIndex++)
				{
					DestPartSys->LODDistances(DistIndex) = DistIndex * 2500.0f;
				}
			}

			if (!DuplicateEmitter(SelectedEmitter, DestPartSys))
			{
				appMsgf(AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_FailedToCopy"), 
					*SelectedEmitter->GetEmitterName().ToString(),
					*DestPartSys->GetName()));
			}

			DestPartSys->MarkPackageDirty();

			// If we temporarily inserted LOD levels into the selected emitter,
			// we need to remove them now...
			if (NewCount > 0)
			{
				INT CurrCount = SelectedEmitter->LODLevels.Num();
				for (INT RemoveIndex = CurrCount - 1; RemoveIndex >= (CurrCount - NewCount); RemoveIndex--)
				{
					SelectedEmitter->LODLevels.Remove(RemoveIndex);
				}
				SelectedEmitter->UpdateModuleLists();
			}

			// Find instances of this particle system and reset them...
			for (TObjectIterator<UParticleSystemComponent> It;It;++It)
			{
				if (It->Template == DestPartSys)
				{
					UParticleSystemComponent* PSysComp = *It;

					// Force a recache of the view relevance
					PSysComp->bIsViewRelevanceDirty = TRUE;
					UBOOL bIsActive = It->bIsActive;
					It->DeactivateSystem();
					It->ResetParticles();
					if (bIsActive)
					{
						It->ActivateSystem();
					}
					It->BeginDeferredReattach();
				}
			}
		}
	}
}

// Delete selected module from selected emitter.
// Module may be used in other Emitters, so we don't destroy it or anything - garbage collection will handle that.
void WxCascade::DeleteSelectedModule(UBOOL bConfirm)
{
	if (!SelectedModule || !SelectedEmitter)
	{
		return;
	}

	if (SelectedModuleIndex == INDEX_NONE)
	{
		return;
	}

	if ((SelectedModuleIndex == INDEX_REQUIREDMODULE) || 
		(SelectedModuleIndex == INDEX_SPAWNMODULE))
	{
		appMsgf(AMT_OK, *LocalizeUnrealEd(TEXT("Cascade_NoDeleteRequiredOrSpawn")));
		return;
	}

	INT	CurrLODSetting	= GetCurrentlySelectedLODLevelIndex();
	if (CurrLODSetting != 0)
	{
		// Don't allow deleting modules if not at highest LOD
		appMsgf(AMT_OK, *LocalizeUnrealEd("Cascade_ModuleDeleteLODWarning"));
		return;
	}

	// If there are differences in the enabled states of the LOD levels for an emitter,
	// prompt the user to ensure they want to delete it...
	UParticleLODLevel* LODLevel = SelectedEmitter->LODLevels(0);
	UParticleModule* CheckModule;
	UBOOL bEnabledStateDifferent = FALSE;
	UBOOL bEnabled = SelectedModule->bEnabled;
	for (INT LODIndex = 1; (LODIndex < SelectedEmitter->LODLevels.Num()) && !bEnabledStateDifferent; LODIndex++)
	{
		LODLevel = SelectedEmitter->LODLevels(LODIndex);
		switch (SelectedModuleIndex)
		{
		case INDEX_TYPEDATAMODULE:
			CheckModule = LODLevel->TypeDataModule;
			break;
		default:
			CheckModule = LODLevel->Modules(SelectedModuleIndex);
			break;
		}

		check(CheckModule);

		if (LODLevel->IsModuleEditable(CheckModule))
		{
			bEnabledStateDifferent = TRUE;
		}
	}

	if ((bConfirm == TRUE) && (bEnabledStateDifferent == TRUE))
	{
		if (appMsgf(AMT_YesNo, *LocalizeUnrealEd("ModuleDeleteConfirm")) == FALSE)
		{
			return;
		}
	}

	BeginTransaction(TEXT("DeleteSelectedModule"));
	ModifySelectedObjects();
	ModifyParticleSystem();

	PartSys->PreEditChange(NULL);

	// Find the module index...
	INT	DeleteModuleIndex	= -1;

	UParticleLODLevel* HighLODLevel	= SelectedEmitter->GetLODLevel(0);
	check(HighLODLevel);
	for (INT ModuleIndex = 0; ModuleIndex < HighLODLevel->Modules.Num(); ModuleIndex++)
	{
		UParticleModule* CheckModule = HighLODLevel->Modules(ModuleIndex);
		if (CheckModule == SelectedModule)
		{
			DeleteModuleIndex = ModuleIndex;
			break;
		}
	}

	if (SelectedModule->IsDisplayedInCurveEd(CurveEd->EdSetup) && !ModuleIsShared(SelectedModule))
	{
		// Remove it from the curve editor!
		SelectedModule->RemoveModuleCurvesFromEditor(CurveEd->EdSetup);
		CurveEd->CurveChanged();
	}

	// Check all the others...
	for (INT LODIndex = 1; LODIndex < SelectedEmitter->LODLevels.Num(); LODIndex++)
	{
		UParticleLODLevel* LODLevel	= SelectedEmitter->GetLODLevel(LODIndex);
		if (LODLevel)
		{
			UParticleModule* Module;

			if (DeleteModuleIndex >= 0)
			{
				Module = LODLevel->Modules(DeleteModuleIndex);
			}
			else
			{
				Module	= LODLevel->TypeDataModule;
			}

			if (Module)
			{
				Module->RemoveModuleCurvesFromEditor(CurveEd->EdSetup);
				CurveEd->CurveChanged();
			}
		}
			
	}
	CurveEd->CurveEdVC->Viewport->Invalidate();

	if (SelectedEmitter)
	{
		UBOOL bNeedsListUpdated = FALSE;

		for (INT LODIndex = 0; LODIndex < SelectedEmitter->LODLevels.Num(); LODIndex++)
		{
			UParticleLODLevel* LODLevel	= SelectedEmitter->GetLODLevel(LODIndex);

			// See if it is in this LODs level...
			UParticleModule* CheckModule;

			if (DeleteModuleIndex >= 0)
			{
				CheckModule = LODLevel->Modules(DeleteModuleIndex);
			}
			else
			{
				CheckModule = LODLevel->TypeDataModule;
			}

			if (CheckModule)
			{
				if (CheckModule->IsA(UParticleModuleTypeDataBase::StaticClass()))
				{
					check(LODLevel->TypeDataModule == CheckModule);
					LODLevel->TypeDataModule = NULL;
				}
				else
				if (CheckModule->IsA(UParticleModuleEventGenerator::StaticClass()))
				{
					LODLevel->EventGenerator = NULL;
				}
				LODLevel->Modules.RemoveItem(CheckModule);
				bNeedsListUpdated = TRUE;
			}
		}

		if (bNeedsListUpdated)
		{
			SelectedEmitter->UpdateModuleLists();
		}
	}
	else
	{
		// Assume that it's in the module dump...
		ModuleDumpList.RemoveItem(SelectedModule);
	}

	PartSys->PostEditChange(NULL);

	EndTransaction(TEXT("DeleteSelectedModule"));

	SetSelectedEmitter(SelectedEmitter);

	EmitterEdVC->Viewport->Invalidate();

	PartSys->MarkPackageDirty();

}

void WxCascade::EnableSelectedModule()
{
	if (!SelectedModule && !SelectedEmitter)
	{
		return;
	}

	INT	CurrLODSetting	= GetCurrentlySelectedLODLevelIndex();
	if (SelectedEmitter->IsLODLevelValid(CurrLODSetting) == FALSE)
	{
		return;
	}

	UParticleLODLevel* DestLODLevel = SelectedEmitter->GetLODLevel(CurrLODSetting);
	if (DestLODLevel->Level == 0)
	{
		// High LOD modules are ALWAYS enabled.
		return;
	}

	UParticleLODLevel* SourceLODLevel = SelectedEmitter->GetLODLevel(DestLODLevel->Level - 1);
	check(SourceLODLevel);

	BeginTransaction(TEXT("EnableSelectedModule"));
	ModifySelectedObjects();
	ModifyParticleSystem();

	PartSys->PreEditChange(NULL);

	if (SelectedModule)
	{
		// Store the index of the selected module...
		// Force copy the source module...
		UParticleModule* NewModule = SelectedModule->GenerateLODModule(SourceLODLevel, DestLODLevel, 100.0f, FALSE, TRUE);
		check(NewModule);

		// Turn off the LOD validity in the original module...
		SelectedModule->LODValidity &= ~(1 << DestLODLevel->Level);

		// Store the new module
		switch (SelectedModuleIndex)
		{
		case INDEX_NONE:
			break;
		case INDEX_REQUIREDMODULE:
			DestLODLevel->RequiredModule = CastChecked<UParticleModuleRequired>(NewModule);
			break;
		case INDEX_SPAWNMODULE:
			DestLODLevel->SpawnModule = CastChecked<UParticleModuleSpawn>(NewModule);
			break;
		case INDEX_TYPEDATAMODULE:
			DestLODLevel->TypeDataModule = NewModule;
			break;
		default:
			DestLODLevel->Modules(SelectedModuleIndex) = NewModule;
			break;
		}

		SelectedModule = NewModule;
	}

	PartSys->PostEditChange(NULL);

	EndTransaction(TEXT("EnableSelectedModule"));

	SetSelectedModule(SelectedEmitter, SelectedModule);

	EmitterEdVC->Viewport->Invalidate();

	PartSys->MarkPackageDirty();

}

void WxCascade::ResetSelectedModule()
{
}

void WxCascade::RefreshSelectedModule()
{
	if (PartSys && SelectedModule && SelectedEmitter)
	{
		SelectedModule->RefreshModule(PartSys->CurveEdSetup, SelectedEmitter, GetCurrentlySelectedLODLevelIndex());
		PropertyWindow->Rebuild();
	}
}

void WxCascade::RefreshAllModules()
{
}

/**
 *	Set the module to an exact duplicate (copy) of the same module in the highest LOD level.
 */
void WxCascade::DupHighestSelectedModule()
{
	if (!SelectedModule && !SelectedEmitter)
	{
		return;
	}

	INT	CurrLODSetting	= GetCurrentlySelectedLODLevelIndex();
	if (SelectedEmitter->IsLODLevelValid(CurrLODSetting) == FALSE)
	{
		return;
	}

	if (CurrLODSetting == 0)
	{
		// High LOD modules don't allow this.
		return;
	}

	UParticleLODLevel* SourceLODLevel = SelectedEmitter->GetLODLevel(0);
	check(SourceLODLevel);
	UParticleModule* HighestModule = SourceLODLevel->GetModuleAtIndex(SelectedModuleIndex);
	if (HighestModule == NULL)
	{
		// Couldn't find the highest module???
		return;
	}

	BeginTransaction(TEXT("DupHighestSelectedModule"));
	ModifySelectedObjects();
	ModifyParticleSystem();

	PartSys->PreEditChange(NULL);

	UBOOL bIsShared = ModuleIsShared(SelectedModule);
	UParticleModule* NewModule = NULL;
	UParticleLODLevel* DestLODLevel;

	// Store the index of the selected module...
	// Force copy the source module...
	DestLODLevel = SelectedEmitter->GetLODLevel(CurrLODSetting);
	NewModule = HighestModule->GenerateLODModule(SourceLODLevel, DestLODLevel, 100.0f, FALSE, TRUE);
	check(NewModule);

	for (INT LODIndex = CurrLODSetting; LODIndex < SelectedEmitter->LODLevels.Num(); LODIndex++)
	{
		DestLODLevel = SelectedEmitter->GetLODLevel(LODIndex);
		if (SelectedModule->IsUsedInLODLevel(LODIndex))
		{
			if (bIsShared == FALSE)
			{
				// Turn off the LOD validity in the original module... only if it wasn't shared!
				SelectedModule->LODValidity &= ~(1 << LODIndex);
			}
			// Turn on the LOD validity in the new module...
			NewModule->LODValidity |= (1 << LODIndex);

			// Store the new module
			switch (SelectedModuleIndex)
			{
			case INDEX_NONE:
				break;
			case INDEX_REQUIREDMODULE:
				DestLODLevel->RequiredModule = CastChecked<UParticleModuleRequired>(NewModule);
				break;
			case INDEX_SPAWNMODULE:
				DestLODLevel->SpawnModule = CastChecked<UParticleModuleSpawn>(NewModule);
				break;
			case INDEX_TYPEDATAMODULE:
				DestLODLevel->TypeDataModule = NewModule;
				break;
			default:
				DestLODLevel->Modules(SelectedModuleIndex) = NewModule;
				break;
			}
		}
	}

	SelectedModule = NewModule;
	if (SelectedEmitter)
	{
		SelectedEmitter->UpdateModuleLists();
	}

	PartSys->PostEditChange(NULL);

	EndTransaction(TEXT("DupHighestSelectedModule"));

	SetSelectedModule(SelectedEmitter, SelectedModule);
	CascadeTouch();

	EmitterEdVC->Viewport->Invalidate();

	PartSys->MarkPackageDirty();
}

/**
 *	Set the module to the same module in the next higher LOD level.
 */
void WxCascade::DupHigherSelectedModule()
{
	if (!SelectedModule && !SelectedEmitter)
	{
		return;
	}

	INT	CurrLODSetting	= GetCurrentlySelectedLODLevelIndex();
	if (SelectedEmitter->IsLODLevelValid(CurrLODSetting) == FALSE)
	{
		return;
	}

	if (CurrLODSetting == 0)
	{
		// High LOD modules don't allow this.
		return;
	}

	UParticleLODLevel* SourceLODLevel = SelectedEmitter->GetLODLevel(CurrLODSetting - 1);
	check(SourceLODLevel);
	UParticleModule* HighModule = SourceLODLevel->GetModuleAtIndex(SelectedModuleIndex);
	if (HighModule == NULL)
	{
		// Couldn't find the highest module???
		return;
	}

	BeginTransaction(TEXT("DupHighSelectedModule"));
	ModifySelectedObjects();
	ModifyParticleSystem();

	PartSys->PreEditChange(NULL);

	UParticleModule* NewModule = NULL;
	UParticleLODLevel* DestLODLevel;

	UBOOL bIsShared = ModuleIsShared(SelectedModule);
	// Store the index of the selected module...
	// Force copy the source module...
	DestLODLevel = SelectedEmitter->GetLODLevel(CurrLODSetting);
	NewModule = HighModule->GenerateLODModule(SourceLODLevel, DestLODLevel, 100.0f, FALSE, TRUE);
	check(NewModule);

	for (INT LODIndex = CurrLODSetting; LODIndex < SelectedEmitter->LODLevels.Num(); LODIndex++)
	{
		DestLODLevel = SelectedEmitter->GetLODLevel(LODIndex);
		if (SelectedModule->IsUsedInLODLevel(LODIndex))
		{
			if (bIsShared == FALSE)
			{
				// Turn off the LOD validity in the original module... only if it wasn't shared!
				SelectedModule->LODValidity &= ~(1 << LODIndex);
			}
			// Turn on the LOD validity int he new module...
			NewModule->LODValidity |= (1 << LODIndex);

			// Store the new module
			switch (SelectedModuleIndex)
			{
			case INDEX_NONE:
				break;
			case INDEX_REQUIREDMODULE:
				DestLODLevel->RequiredModule = CastChecked<UParticleModuleRequired>(NewModule);
				break;
			case INDEX_SPAWNMODULE:
				DestLODLevel->SpawnModule = CastChecked<UParticleModuleSpawn>(NewModule);
				break;
			case INDEX_TYPEDATAMODULE:
				DestLODLevel->TypeDataModule = NewModule;
				break;
			default:
				DestLODLevel->Modules(SelectedModuleIndex) = NewModule;
				break;
			}
		}
	}

	SelectedModule = NewModule;
	if (SelectedEmitter)
	{
		SelectedEmitter->UpdateModuleLists();
	}

	PartSys->PostEditChange(NULL);

	SetSelectedModule(SelectedEmitter, SelectedModule);

	EndTransaction(TEXT("DupHighSelectedModule"));
	CascadeTouch();

	PartSys->MarkPackageDirty();
	EmitterEdVC->Viewport->Invalidate();
}

void WxCascade::ShareHigherSelectedModule()
{
	if (!SelectedModule && !SelectedEmitter)
	{
		return;
	}

	INT	CurrLODSetting	= GetCurrentlySelectedLODLevelIndex();
	if (SelectedEmitter->IsLODLevelValid(CurrLODSetting) == FALSE)
	{
		return;
	}

	if (CurrLODSetting == 0)
	{
		// High LOD modules don't allow this.
		return;
	}

	UParticleLODLevel* SourceLODLevel = SelectedEmitter->GetLODLevel(CurrLODSetting - 1);
	check(SourceLODLevel);
	UParticleModule* HighModule = SourceLODLevel->GetModuleAtIndex(SelectedModuleIndex);
	if (HighModule == NULL)
	{
		// Couldn't find the highest module???
		return;
	}

	BeginTransaction(TEXT("ShareHigherSelectedModule"));
	ModifySelectedObjects();
	ModifyParticleSystem();

	PartSys->PreEditChange(NULL);

	UParticleModule* NewModule = NULL;
	UParticleLODLevel* DestLODLevel;
	UBOOL bIsShared = ModuleIsShared(SelectedModule);
	// Store the index of the selected module...
	// Force copy the source module...
	DestLODLevel = SelectedEmitter->GetLODLevel(CurrLODSetting);
	NewModule = HighModule->GenerateLODModule(SourceLODLevel, DestLODLevel, 100.0f, FALSE, FALSE);
	check(NewModule);

	for (INT LODIndex = CurrLODSetting; LODIndex < SelectedEmitter->LODLevels.Num(); LODIndex++)
	{
		DestLODLevel = SelectedEmitter->GetLODLevel(LODIndex);
		if (SelectedModule->IsUsedInLODLevel(LODIndex))
		{
			if (bIsShared == FALSE)
			{
				// Turn off the LOD validity in the original module...
				SelectedModule->LODValidity &= ~(1 << DestLODLevel->Level);
			}
			// Turn on the LOD validity int he new module...
			NewModule->LODValidity |= (1 << LODIndex);

			// Store the new module
			switch (SelectedModuleIndex)
			{
			case INDEX_NONE:
				break;
			case INDEX_REQUIREDMODULE:
				DestLODLevel->RequiredModule = CastChecked<UParticleModuleRequired>(NewModule);
				break;
			case INDEX_SPAWNMODULE:
				DestLODLevel->SpawnModule = CastChecked<UParticleModuleSpawn>(NewModule);
				break;
			case INDEX_TYPEDATAMODULE:
				DestLODLevel->TypeDataModule = NewModule;
				break;
			default:
				DestLODLevel->Modules(SelectedModuleIndex) = NewModule;
				break;
			}
		}
	}

	SelectedModule = NewModule;
	if (SelectedEmitter)
	{
		SelectedEmitter->UpdateModuleLists();
	}

	PartSys->PostEditChange(NULL);

	CascadeTouch();
	SetSelectedModule(SelectedEmitter, SelectedModule);

	EndTransaction(TEXT("ShareHigherSelectedModule"));

	PartSys->MarkPackageDirty();
	EmitterEdVC->Viewport->Invalidate();
}

UBOOL WxCascade::ModuleIsShared(UParticleModule* InModule)
{
	INT FindCount = 0;

	UParticleModuleSpawn* SpawnModule = Cast<UParticleModuleSpawn>(InModule);
	UParticleModuleRequired* RequiredModule = Cast<UParticleModuleRequired>(InModule);
	UParticleModuleTypeDataBase* TypeDataModule = Cast<UParticleModuleTypeDataBase>(InModule);

	INT	CurrLODSetting	= GetCurrentlySelectedLODLevelIndex();
	if (CurrLODSetting < 0)
	{
		return FALSE;
	}

	for (INT i = 0; i < PartSys->Emitters.Num(); i++)
	{
		UParticleEmitter* Emitter = PartSys->Emitters(i);
		UParticleLODLevel* LODLevel = Emitter->GetLODLevel(CurrLODSetting);
		if (LODLevel == NULL)
		{
			continue;
		}

		if (SpawnModule)
		{
			if (SpawnModule == LODLevel->SpawnModule)
			{
				FindCount++;
				if (FindCount >= 2)
				{
					return TRUE;
				}
			}
		}
		else if (RequiredModule)
		{
			if (RequiredModule == LODLevel->RequiredModule)
            {
                FindCount++;
                if (FindCount >= 2)
                {
	                return TRUE;
                }
            }
		}
		else if (TypeDataModule)
		{
			if (TypeDataModule == LODLevel->TypeDataModule)
			{
				FindCount++;
				if (FindCount >= 2)
				{
					return TRUE;
				}
			}
		}
		else
		{
			for (INT j = 0; j < LODLevel->Modules.Num(); j++)
			{
				if (LODLevel->Modules(j) == InModule)
				{
					FindCount++;
					if (FindCount == 2)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

void WxCascade::AddSelectedToGraph()
{
	if (!SelectedEmitter)
		return;

	INT	CurrLODSetting	= GetCurrentlySelectedLODLevelIndex();
	if (SelectedEmitter->IsLODLevelValid(CurrLODSetting) == FALSE)
	{
		return;
	}

	if (SelectedModule)
	{
		UParticleLODLevel* LODLevel = SelectedEmitter->GetLODLevel(CurrLODSetting);
		if (LODLevel->IsModuleEditable(SelectedModule))
		{
			SelectedModule->AddModuleCurvesToEditor( PartSys->CurveEdSetup );
			CurveEd->CurveChanged();
		}
	}

	SetSelectedInCurveEditor();
	CurveEd->CurveEdVC->Viewport->Invalidate();
}

void WxCascade::SetSelectedInCurveEditor()
{
	CurveEd->ClearAllSelectedCurves();
	if (SelectedModule)
	{
		TArray<FParticleCurvePair> Curves;
		SelectedModule->GetCurveObjects(Curves);
		for (INT CurveIndex = 0; CurveIndex < Curves.Num(); CurveIndex++)
		{
			UObject* Distribution = Curves(CurveIndex).CurveObject;
			if (Distribution)
			{
				CurveEd->SetCurveSelected(Distribution, TRUE);
			}
		}
		CurveEd->ScrollToFirstSelected();
	}
	CurveEd->CurveEdVC->Viewport->Invalidate();
}

void WxCascade::SetCopyEmitter(UParticleEmitter* NewEmitter)
{
	CopyEmitter = NewEmitter;
}

void WxCascade::SetCopyModule(UParticleEmitter* NewEmitter, UParticleModule* NewModule)
{
	CopyEmitter = NewEmitter;
	CopyModule = NewModule;
}

void WxCascade::RemoveModuleFromDump(UParticleModule* Module)
{
	for (INT i = 0; i < ModuleDumpList.Num(); i++)
	{
		if (ModuleDumpList(i) == Module)
		{
			ModuleDumpList.Remove(i);
			break;
		}
	}
}

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxCascade::GetDockingParentName() const
{
	return TEXT("Cascade");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxCascade::GetDockingParentVersion() const
{
	return 0;
}


//
// UCascadeOptions
// 
