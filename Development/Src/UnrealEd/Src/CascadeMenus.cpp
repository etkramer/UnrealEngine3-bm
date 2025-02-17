/*=============================================================================
	CascadeMenus.cpp: 'Cascade' particle editor menus
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "Cascade.h"

/*-----------------------------------------------------------------------------
	WxCascadeMenuBar
-----------------------------------------------------------------------------*/

WxCascadeMenuBar::WxCascadeMenuBar(WxCascade* Cascade)
{
	EditMenu = new wxMenu();
	Append( EditMenu, *LocalizeUnrealEd("Edit") );
#if 0
	EditMenu->Append(IDM_CASCADE_RESET_PEAK_COUNTS,		*LocalizeUnrealEd("ResetPeakCounts"),		TEXT("") );
	EditMenu->AppendSeparator();
	EditMenu->Append(IDM_CASCADE_CONVERT_TO_UBER,		*LocalizeUnrealEd("UberConvert"),			TEXT("") );
	EditMenu->AppendSeparator();
#endif
	EditMenu->Append(IDM_CASCADE_REGENERATE_LOWESTLOD,	*LocalizeUnrealEd("CascadeRegenLowestLOD"),	TEXT("") );
	EditMenu->AppendSeparator();
	EditMenu->Append(IDM_CASCADE_SAVE_PACKAGE,			*LocalizeUnrealEd("CascadeSavePackage"),	TEXT("") );

	ViewMenu = new wxMenu();
	Append( ViewMenu, *LocalizeUnrealEd("View") );

	ViewMenu->AppendCheckItem( IDM_CASCADE_VIEW_AXES, *LocalizeUnrealEd("ViewOriginAxes"), TEXT("") );
	ViewMenu->AppendCheckItem(IDM_CASCADE_VIEW_COUNTS, *LocalizeUnrealEd("ViewParticleCounts"), TEXT(""));
	ViewMenu->Check(IDM_CASCADE_VIEW_COUNTS, Cascade->PreviewVC->bDrawParticleCounts == TRUE);
	ViewMenu->AppendCheckItem(IDM_CASCADE_VIEW_EVENTS, *LocalizeUnrealEd("ViewParticleEvents"), TEXT(""));
	ViewMenu->Check(IDM_CASCADE_VIEW_EVENTS, Cascade->PreviewVC->bDrawParticleEvents == TRUE);
	ViewMenu->AppendCheckItem(IDM_CASCADE_VIEW_TIMES, *LocalizeUnrealEd("ViewParticleTimes"), TEXT(""));
	ViewMenu->Check(IDM_CASCADE_VIEW_TIMES, Cascade->PreviewVC->bDrawParticleTimes == TRUE);
	ViewMenu->AppendCheckItem(IDM_CASCADE_VIEW_DISTANCE, *LocalizeUnrealEd("ViewParticleDistance"), TEXT(""));
	ViewMenu->Check(IDM_CASCADE_VIEW_DISTANCE, Cascade->PreviewVC->bDrawSystemDistance == TRUE);
	ViewMenu->AppendCheckItem(IDM_CASCADE_VIEW_GEOMETRY, *LocalizeUnrealEd("ViewParticleGeometry"), TEXT(""));
	ViewMenu->Check(IDM_CASCADE_VIEW_GEOMETRY, Cascade->EditorOptions->bShowFloor == TRUE);
	ViewMenu->Append(IDM_CASCADE_VIEW_GEOMETRY_PROPERTIES, *LocalizeUnrealEd("Cascade_ViewGeometryProperties"),	TEXT("") );
	ViewMenu->AppendSeparator();
	ViewMenu->Append( IDM_CASCADE_SAVECAM, *LocalizeUnrealEd("SaveCamPosition"), TEXT("") );
#if defined(_CASCADE_ENABLE_MODULE_DUMP_)
	ViewMenu->AppendCheckItem(IDM_CASCADE_VIEW_DUMP, *LocalizeUnrealEd("ViewModuleDump"), TEXT(""));
	ViewMenu->Check(IDM_CASCADE_VIEW_DUMP, Cascade->EditorOptions->bShowModuleDump == TRUE);
#endif	//#if defined(_CASCADE_ENABLE_MODULE_DUMP_)
	ViewMenu->AppendSeparator();
}

WxCascadeMenuBar::~WxCascadeMenuBar()
{

}

/*-----------------------------------------------------------------------------
	WxMBCascadeModule
-----------------------------------------------------------------------------*/

WxMBCascadeModule::WxMBCascadeModule(WxCascade* Cascade)
{
	Append(IDM_CASCADE_DELETE_MODULE,	*LocalizeUnrealEd("DeleteModule"),	TEXT(""));
	Append(IDM_CASCADE_REFRESH_MODULE, *LocalizeUnrealEd("Casc_RefreshModule"), TEXT(""));

	if (Cascade->SelectedModule)
	{
		if (Cascade->SelectedModule->IsA(UParticleModuleRequired::StaticClass()))
		{
			AppendSeparator();
			Append(IDM_CASCADE_MODULE_SYNCMATERIAL,	*LocalizeUnrealEd("Casc_SyncMaterial"),	TEXT(""));
			Append(IDM_CASCADE_MODULE_USEMATERIAL,	*LocalizeUnrealEd("Casc_UseMaterial"),	TEXT(""));
		}
	}
	INT CurrLODLevel = Cascade->GetCurrentlySelectedLODLevelIndex();
	if (CurrLODLevel > 0)
	{
		AppendSeparator();
		Append(IDM_CASCADE_MODULE_DUPHIGH,		*LocalizeUnrealEd("DuplicateFromHigher"),	TEXT(""));
		Append(IDM_CASCADE_MODULE_SHAREHIGH,	*LocalizeUnrealEd("ShareWithHigher"),		TEXT(""));
		Append(IDM_CASCADE_MODULE_DUPHIGHEST,	*LocalizeUnrealEd("DuplicateFromHighest"),	TEXT(""));
	}
}

WxMBCascadeModule::~WxMBCascadeModule()
{

}

/*-----------------------------------------------------------------------------
	WxMBCascadeEmitterBkg
-----------------------------------------------------------------------------*/
UBOOL WxMBCascadeEmitterBkg::InitializedModuleEntries = FALSE;
TArray<FString>	WxMBCascadeEmitterBkg::TypeDataModuleEntries;
TArray<INT>		WxMBCascadeEmitterBkg::TypeDataModuleIndices;
TArray<FString>	WxMBCascadeEmitterBkg::ModuleEntries;
TArray<INT>		WxMBCascadeEmitterBkg::ModuleIndices;

WxMBCascadeEmitterBkg::WxMBCascadeEmitterBkg(WxCascade* Cascade, WxMBCascadeEmitterBkg::Mode eMode)
{
	// Don't show options if no emitter selected.
	if (!Cascade->SelectedEmitter)
		return;

	EmitterMenu = 0;
	PSysMenu = NULL;
	SelectedModuleMenu = 0;
	TypeDataMenu = 0;
	NonTypeDataMenus.Empty();

	InitializeModuleEntries(Cascade);

	check(TypeDataModuleEntries.Num() == TypeDataModuleIndices.Num());
	check(ModuleEntries.Num() == ModuleIndices.Num());

	AddEmitterEntries(Cascade, eMode);
	AddSelectedModuleEntries(Cascade, eMode);
	AddPSysEntries(Cascade, eMode);
	AddTypeDataEntries(Cascade, eMode);
	AddNonTypeDataEntries(Cascade, eMode);
}

WxMBCascadeEmitterBkg::~WxMBCascadeEmitterBkg()
{

}

void WxMBCascadeEmitterBkg::InitializeModuleEntries(WxCascade* Cascade)
{
    INT i;
    UBOOL bFoundTypeData = FALSE;
	UParticleModule* DefModule;

	if (!InitializedModuleEntries)
	{
		TypeDataModuleEntries.Empty();
		TypeDataModuleIndices.Empty();
		ModuleEntries.Empty();
		ModuleIndices.Empty();

		// add the data type modules to the menu
		for(i = 0; i < Cascade->ParticleModuleClasses.Num(); i++)
		{
			DefModule = (UParticleModule*)Cascade->ParticleModuleClasses(i)->GetDefaultObject();

			if (Cascade->ParticleModuleClasses(i)->IsChildOf(UParticleModuleTypeDataBase::StaticClass()))
			{
				bFoundTypeData = TRUE;
				FString NewModuleString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("New_F"), *Cascade->ParticleModuleClasses(i)->GetDescription()) );
				TypeDataModuleEntries.AddItem(NewModuleString);
				TypeDataModuleIndices.AddItem(i);
			}
		}

		// Add each module type to menu.
		for(i = 0; i < Cascade->ParticleModuleClasses.Num(); i++)
		{
			DefModule = (UParticleModule*)Cascade->ParticleModuleClasses(i)->GetDefaultObject();

			if (Cascade->ParticleModuleClasses(i)->IsChildOf(UParticleModuleTypeDataBase::StaticClass()) == FALSE)
			{
				FString NewModuleString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("New_F"), *Cascade->ParticleModuleClasses(i)->GetDescription()) );
				ModuleEntries.AddItem(NewModuleString);
				ModuleIndices.AddItem(i);
			}
		}
		InitializedModuleEntries = TRUE;
	}
}

void WxMBCascadeEmitterBkg::AddEmitterEntries(WxCascade* Cascade, WxMBCascadeEmitterBkg::Mode eMode)
{
	if ((UINT)eMode & EMITTER_ONLY)
	{
		if (Cascade->EditorOptions->bUseSubMenus == FALSE)
		{
			Append(IDM_CASCADE_RENAME_EMITTER, *LocalizeUnrealEd("RenameEmitter"), TEXT(""));
			Append(IDM_CASCADE_DUPLICATE_EMITTER, *LocalizeUnrealEd("DuplicateEmitter"), TEXT(""));
			Append(IDM_CASCADE_DUPLICATE_SHARE_EMITTER, *LocalizeUnrealEd("DuplicateShareEmitter"), TEXT(""));
			Append(IDM_CASCADE_DELETE_EMITTER, *LocalizeUnrealEd("DeleteEmitter"), TEXT(""));
			Append(IDM_CASCADE_EXPORT_EMITTER, *LocalizeUnrealEd("ExportEmitter"), TEXT(""));
			if (GIsEpicInternal)
			{
				AppendSeparator();
				Append(IDM_CASCADE_CONVERT_RAIN_EMITTER, TEXT("Uber-rain"), TEXT(""));
			}
		}
		else
		{
			EmitterMenu = new wxMenu();
			EmitterMenu->Append(IDM_CASCADE_RENAME_EMITTER, *LocalizeUnrealEd("RenameEmitter"), TEXT(""));
			EmitterMenu->Append(IDM_CASCADE_DUPLICATE_EMITTER, *LocalizeUnrealEd("DuplicateEmitter"), TEXT(""));
			EmitterMenu->Append(IDM_CASCADE_DUPLICATE_SHARE_EMITTER, *LocalizeUnrealEd("DuplicateShareEmitter"), TEXT(""));
			EmitterMenu->Append(IDM_CASCADE_DELETE_EMITTER, *LocalizeUnrealEd("DeleteEmitter"), TEXT(""));
			EmitterMenu->Append(IDM_CASCADE_EXPORT_EMITTER, *LocalizeUnrealEd("ExportEmitter"), TEXT(""));
			if (GIsEpicInternal)
			{
				EmitterMenu->AppendSeparator();
				EmitterMenu->Append(IDM_CASCADE_CONVERT_RAIN_EMITTER, TEXT("Uber-rain"), TEXT(""));
			}
			Append(IDMENU_CASCADE_POPUP_EMITTER, *LocalizeUnrealEd("Emitter"), EmitterMenu);
		}

		AppendSeparator();

	}
}

void WxMBCascadeEmitterBkg::AddPSysEntries(WxCascade* Cascade, Mode eMode)
{
	if ((UINT)eMode & PSYS_ONLY)
	{
		if (Cascade->EditorOptions->bUseSubMenus == FALSE)
		{
			Append(IDM_CASCADE_SELECT_PARTICLESYSTEM, *LocalizeUnrealEd("SelectParticleSystem"), TEXT(""));
			if (Cascade->SelectedEmitter != NULL)
			{
				Append(IDM_CASCADE_PSYS_NEW_EMITTER_BEFORE, *LocalizeUnrealEd("NewEmitterLeft"), TEXT(""));
				Append(IDM_CASCADE_PSYS_NEW_EMITTER_AFTER, *LocalizeUnrealEd("NewEmitterRight"), TEXT(""));
			}
		}
		else
		{
			PSysMenu = new wxMenu();
			PSysMenu->Append(IDM_CASCADE_SELECT_PARTICLESYSTEM, *LocalizeUnrealEd("SelectParticleSystem"), TEXT(""));
			if (Cascade->SelectedEmitter != NULL)
			{
				PSysMenu->Append(IDM_CASCADE_PSYS_NEW_EMITTER_BEFORE, *LocalizeUnrealEd("NewEmitterLeft"), TEXT(""));
				PSysMenu->Append(IDM_CASCADE_PSYS_NEW_EMITTER_AFTER, *LocalizeUnrealEd("NewEmitterRight"), TEXT(""));
			}
			Append(IDMENU_CASCADE_POPUP_PSYS, *LocalizeUnrealEd("PSys"), PSysMenu);
		}

		AppendSeparator();
	}
}

void WxMBCascadeEmitterBkg::AddSelectedModuleEntries(WxCascade* Cascade, WxMBCascadeEmitterBkg::Mode eMode)
{
}

void WxMBCascadeEmitterBkg::AddTypeDataEntries(WxCascade* Cascade, WxMBCascadeEmitterBkg::Mode eMode)
{
	if ((UINT)eMode & TYPEDATAS_ONLY)
	{
		if (TypeDataModuleEntries.Num())
		{
			if (Cascade->EditorOptions->bUseSubMenus == FALSE)
			{
				// add the data type modules to the menu
				for (INT i = 0; i < TypeDataModuleEntries.Num(); i++)
				{
					Append(IDM_CASCADE_NEW_MODULE_START + TypeDataModuleIndices(i), 
						*TypeDataModuleEntries(i), TEXT(""));
				}
			}
			else
			{
				TypeDataMenu = new wxMenu();
				for (INT i = 0; i < TypeDataModuleEntries.Num(); i++)
				{
					TypeDataMenu->Append(IDM_CASCADE_NEW_MODULE_START + TypeDataModuleIndices(i), 
						*TypeDataModuleEntries(i), TEXT(""));
				}
				Append(IDMENU_CASCADE_POPUP_TYPEDATA, *LocalizeUnrealEd("TypeData"), TypeDataMenu);
			}
			AppendSeparator();
		}
	}
}

void WxMBCascadeEmitterBkg::AddNonTypeDataEntries(WxCascade* Cascade, WxMBCascadeEmitterBkg::Mode eMode)
{
	if ((UINT)eMode & NON_TYPEDATAS)
	{
		if (ModuleEntries.Num())
		{
			if (Cascade->EditorOptions->bUseSubMenus == FALSE)
			{
				// Add each module type to menu.
				for (INT i = 0; i < ModuleEntries.Num(); i++)
				{
					Append(IDM_CASCADE_NEW_MODULE_START + ModuleIndices(i), *ModuleEntries(i), TEXT(""));
				}
			}
			else
			{
				UBOOL bFoundTypeData = FALSE;
				INT i, j;
				INT iIndex = 0;
				FString ModuleName;

				// Now, for each module base type, add another branch
				for (i = 0; i < Cascade->ParticleModuleBaseClasses.Num(); i++)
				{
					ModuleName = Cascade->ParticleModuleBaseClasses(i)->GetName();
					if (Cascade->IsModuleSuitableForModuleMenu(ModuleName) &&
						Cascade->IsBaseModuleTypeDataPairSuitableForModuleMenu(ModuleName))
					{
						// Add the 'label'
						wxMenu* pkNewMenu = new wxMenu();

						// Search for all modules of this type
						for (j = 0; j < Cascade->ParticleModuleClasses.Num(); j++)
						{
							if (Cascade->ParticleModuleClasses(j)->IsChildOf(Cascade->ParticleModuleBaseClasses(i)))
							{
								ModuleName = Cascade->ParticleModuleClasses(j)->GetName();
								if (Cascade->IsModuleSuitableForModuleMenu(ModuleName) &&
									Cascade->IsModuleTypeDataPairSuitableForModuleMenu(ModuleName))
								{
									pkNewMenu->Append(
										IDM_CASCADE_NEW_MODULE_START + j, 
										*(Cascade->ParticleModuleClasses(j)->GetDescription()), TEXT(""));
								}
							}
						}

						Append(IDMENU_CASCADE_POPUP_NONTYPEDATA_START + iIndex, 
							*(Cascade->ParticleModuleBaseClasses(i)->GetDescription()), pkNewMenu);
						NonTypeDataMenus.AddItem(pkNewMenu);

						iIndex++;
						check(IDMENU_CASCADE_POPUP_NONTYPEDATA_START + iIndex < IDMENU_CASCADE_POPUP_NONTYPEDATA_END);
					}
				}
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	WxMBCascadeBkg
-----------------------------------------------------------------------------*/

WxMBCascadeBkg::WxMBCascadeBkg(WxCascade* Cascade)
{
	UParticleEmitter* DefEmitter = Cast<UParticleEmitter>(UParticleSpriteEmitter::StaticClass()->GetDefaultObject());
	FString NewModuleString = FString::Printf( LocalizeSecure(LocalizeUnrealEd("New_F"), *(UParticleSpriteEmitter::StaticClass()->GetDescription())) );
	Append(IDM_CASCADE_NEW_EMITTER_START, *NewModuleString, TEXT(""));
	if (Cascade->SelectedEmitter)
	{
		AppendSeparator();
		// Only put the delete option in if there is a selected emitter!
		Append( IDM_CASCADE_DELETE_EMITTER, *LocalizeUnrealEd("DeleteEmitter"), TEXT("") );
	}
}

WxMBCascadeBkg::~WxMBCascadeBkg()
{

}

/*-----------------------------------------------------------------------------
	WxCascadePostProcessMenu
-----------------------------------------------------------------------------*/
WxCascadePostProcessMenu::WxCascadePostProcessMenu(WxCascade* Cascade)
{
	ShowPPFlagData.AddItem(
		FCascShowPPFlagData(
			IDM_CASC_SHOWPP_BLOOM, 
			*LocalizeUnrealEd("Cascade_ShowBloom"), 
			CASC_SHOW_BLOOM
			)
		);
	ShowPPFlagData.AddItem(
		FCascShowPPFlagData(
			IDM_CASC_SHOWPP_DOF, 
			*LocalizeUnrealEd("Cascade_ShowDOF"), 
			CASC_SHOW_DOF
			)
		);
	ShowPPFlagData.AddItem(
		FCascShowPPFlagData(
			IDM_CASC_SHOWPP_MOTIONBLUR, 
			*LocalizeUnrealEd("Cascade_ShowMotionBlur"), 
			CASC_SHOW_MOTIONBLUR
			)
		);
	ShowPPFlagData.AddItem(
		FCascShowPPFlagData(
			IDM_CASC_SHOWPP_PPVOLUME, 
			*LocalizeUnrealEd("Cascade_ShowPPVolumeMaterial"), 
			CASC_SHOW_PPVOLUME
			)
		);

	for (INT i = 0; i < ShowPPFlagData.Num(); ++i)
	{
		const FCascShowPPFlagData& ShowFlagData = ShowPPFlagData(i);

		AppendCheckItem(ShowFlagData.ID, *ShowFlagData.Name);
		if (Cascade->PreviewVC)
		{
			if ((Cascade->PreviewVC->ShowPPFlags & ShowFlagData.Mask) != 0)
			{
				Check(ShowFlagData.ID, TRUE);
			}
			else
			{
				Check(ShowFlagData.ID, FALSE);
			}
		}
		else
		{
			Check(ShowFlagData.ID, FALSE);
		}
	}
}

WxCascadePostProcessMenu::~WxCascadePostProcessMenu()
{
}

/*-----------------------------------------------------------------------------
	WxCascadeToolBar
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxCascadeToolBar, wxToolBar )
END_EVENT_TABLE()

WxCascadeToolBar::WxCascadeToolBar( wxWindow* InParent, wxWindowID InID )
: wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
{
	Cascade	= (WxCascade*)InParent;

	AddSeparator();

	SaveCamB.Load( TEXT("CASC_SaveCam") );
	ResetSystemB.Load( TEXT("CASC_ResetSystem") );
	RestartInLevelB.Load( TEXT("CASC_ResetInLevel") );
	OrbitModeB.Load(TEXT("CASC_OrbitMode"));
	WireframeB.Load(TEXT("CASC_Wireframe"));
	BoundsB.Load(TEXT("CASC_Bounds"));
	PostProcessB.Load(TEXT("CASC_PostProcess"));
	ToggleGridB.Load(TEXT("CASC_ToggleGrid"));

	BackgroundColorB.Load(TEXT("CASC_BackColor"));
	WireSphereB.Load(TEXT("Sphere"));

	PerformanceMeterB.Load(TEXT("CASC_PerformanceCheck"));

	UndoB.Load(TEXT("CASC_Undo"));
	RedoB.Load(TEXT("CASC_Redo"));
    
	PlayB.Load(TEXT("CASC_Speed_Play"));
	PauseB.Load(TEXT("CASC_Speed_Pause"));
	Speed1B.Load(TEXT("CASC_Speed_1"));
	Speed10B.Load(TEXT("CASC_Speed_10"));
	Speed25B.Load(TEXT("CASC_Speed_25"));
	Speed50B.Load(TEXT("CASC_Speed_50"));
	Speed100B.Load(TEXT("CASC_Speed_100"));

	LoopSystemB.Load(TEXT("CASC_LoopSystem"));

	LODHigh.Load(TEXT("CASC_LODHigh"));
	LODHigher.Load(TEXT("CASC_LODHigher"));
	LODLower.Load(TEXT("CASC_LODLower"));
	LODLow.Load(TEXT("CASC_LODLow"));

	LODAddBefore.Load(TEXT("CASC_LODAddBefore"));
	LODAddAfter.Load(TEXT("CASC_LODAddAfter"));

	LODDelete.Load(TEXT("CASC_LODDelete"));
	LODRegenerate.Load(TEXT("CASC_LODRegen"));
	LODRegenerateDuplicate.Load(TEXT("CASC_LODRegenDup"));

	RealtimeB.Load(TEXT("CASC_Realtime"));

	SetToolBitmapSize( wxSize( 18, 18 ) );

	AddTool( IDM_CASCADE_RESETSYSTEM, ResetSystemB, *LocalizeUnrealEd("RestartSim") );
	AddTool(IDM_CASCADE_RESETINLEVEL, RestartInLevelB, *LocalizeUnrealEd("RestartInLevel"));
	//AddCheckTool( IDM_CASCADE_SIM_RESTARTONFINISH, ResetSystemB, NULL, *LocalizeUnrealEd("LoopSimulation") )
	AddSeparator();
	AddTool( IDM_CASCADE_SAVECAM, SaveCamB, *LocalizeUnrealEd("SaveCameraPosition") );
	AddSeparator();
	AddCheckTool(IDM_CASCADE_ORBITMODE, *LocalizeUnrealEd("ToggleOrbitMode"), OrbitModeB, wxNullBitmap, *LocalizeUnrealEd("ToggleOrbitMode"));
	ToggleTool(IDM_CASCADE_ORBITMODE, TRUE);
	AddCheckTool(IDM_CASCADE_WIREFRAME, *LocalizeUnrealEd("ToggleWireframe"), WireframeB, wxNullBitmap, *LocalizeUnrealEd("ToggleWireframe"));
	ToggleTool(IDM_CASCADE_WIREFRAME, FALSE);
	AddCheckTool(IDM_CASCADE_BOUNDS, *LocalizeUnrealEd("ToggleBounds"), BoundsB, wxNullBitmap, *LocalizeUnrealEd("ToggleBounds"));
	ToggleTool(IDM_CASCADE_BOUNDS, FALSE);
	AddTool(IDM_CASCADE_POSTPROCESS, PostProcessB, *LocalizeUnrealEd("TogglePostProcess"));
	AddCheckTool(IDM_CASCADE_TOGGLEGRID, *LocalizeUnrealEd("Casc_ToggleGrid"), ToggleGridB, wxNullBitmap, *LocalizeUnrealEd("Casc_ToggleGrid"));
	ToggleTool(IDM_CASCADE_TOGGLEGRID, TRUE);

	AddSeparator();

	AddRadioTool(IDM_CASCADE_PLAY, *LocalizeUnrealEd("Play"), PlayB, wxNullBitmap, *LocalizeUnrealEd("Play"));
	AddRadioTool(IDM_CASCADE_PAUSE, *LocalizeUnrealEd("Pause"), PauseB, wxNullBitmap, *LocalizeUnrealEd("Pause"));
	ToggleTool(IDM_CASCADE_PLAY, TRUE);

	AddSeparator();

	AddRadioTool(IDM_CASCADE_SPEED_100,	*LocalizeUnrealEd("FullSpeed"), Speed100B, wxNullBitmap, *LocalizeUnrealEd("FullSpeed"));
	AddRadioTool(IDM_CASCADE_SPEED_50,	*LocalizeUnrealEd("50Speed"), Speed50B, wxNullBitmap, *LocalizeUnrealEd("50Speed"));
	AddRadioTool(IDM_CASCADE_SPEED_25,	*LocalizeUnrealEd("25Speed"), Speed25B, wxNullBitmap, *LocalizeUnrealEd("25Speed"));
	AddRadioTool(IDM_CASCADE_SPEED_10,	*LocalizeUnrealEd("10Speed"), Speed10B, wxNullBitmap, *LocalizeUnrealEd("10Speed"));
	AddRadioTool(IDM_CASCADE_SPEED_1,	*LocalizeUnrealEd("1Speed"), Speed1B, wxNullBitmap, *LocalizeUnrealEd("1Speed"));
	ToggleTool(IDM_CASCADE_SPEED_100, TRUE);

	AddSeparator();

	AddCheckTool(IDM_CASCADE_LOOPSYSTEM, *LocalizeUnrealEd("ToggleLoopSystem"), LoopSystemB, wxNullBitmap, *LocalizeUnrealEd("ToggleLoopSystem"));
	ToggleTool(IDM_CASCADE_LOOPSYSTEM, TRUE);

	AddCheckTool(IDM_CASCADE_REALTIME, *LocalizeUnrealEd("ToggleRealtime"), RealtimeB, wxNullBitmap, *LocalizeUnrealEd("ToggleRealtime"));
	ToggleTool(IDM_CASCADE_REALTIME, TRUE);
	bRealtime	= TRUE;

	AddSeparator();
	AddTool(IDM_CASCADE_BACKGROUND_COLOR, BackgroundColorB, *LocalizeUnrealEd("BackgroundColor"));
	AddCheckTool(IDM_CASCADE_TOGGLE_WIRE_SPHERE, *LocalizeUnrealEd("CascadeToggleWireSphere"), WireSphereB, wxNullBitmap, *LocalizeUnrealEd("CascadeToggleWireSphere"));

	AddSeparator();
	AddTool(IDM_CASCADE_UNDO, UndoB, *LocalizeUnrealEd("Undo"));
	AddTool(IDM_CASCADE_REDO, RedoB, *LocalizeUnrealEd("Redo"));

	AddSeparator();
	AddTool(IDM_CASCADE_PERFORMANCE_CHECK, PerformanceMeterB, *LocalizeUnrealEd("CascadePerfCheck"));

	AddSeparator();
	AddSeparator();
	if (GIsEpicInternal)
	{
		AddTool(IDM_CASCADE_LOD_REGENDUP,	LODRegenerateDuplicate,	*LocalizeUnrealEd("CascadeLODRegenerateDuplicate"));
		AddTool(IDM_CASCADE_LOD_REGEN,		LODRegenerate,			*LocalizeUnrealEd("CascadeLODRegenerate"));
	}
	AddSeparator();

	LODCurrent = new wxTextCtrl(this, IDM_CASCADE_LOD_CURRENT, TEXT("0"));
	check(LODCurrent);
	wxSize CurrentToolSize = LODCurrent->GetSize();
	LODCurrent->SetSize(CurrentToolSize.GetWidth() / 4, CurrentToolSize.GetHeight());
	LODCurrent->SetEditable(FALSE);
	LODTotal = new wxTextCtrl(this, IDM_CASCADE_LOD_TOTAL, TEXT("of -1"));
	check(LODTotal);
	LODTotal->SetSize(CurrentToolSize.GetWidth() / 2, CurrentToolSize.GetHeight());
	LODTotal->SetEditable(FALSE);

	AddTool(IDM_CASCADE_LOD_HIGH,		LODHigh,		*LocalizeUnrealEd("CascadeLODHigh"));
	AddTool(IDM_CASCADE_LOD_HIGHER,		LODHigher,		*LocalizeUnrealEd("CascadeLODHigher"));
	AddTool(IDM_CASCADE_LOD_ADDBEFORE,	LODAddBefore,	*LocalizeUnrealEd("CascadeLODAddBefore"));
	AddControl(LODCurrent);
	AddControl(LODTotal);
	AddTool(IDM_CASCADE_LOD_ADDAFTER,	LODAddAfter,	*LocalizeUnrealEd("CascadeLODAddAfter"));
	AddTool(IDM_CASCADE_LOD_LOWER,		LODLower,		*LocalizeUnrealEd("CascadeLODLower"));
	AddTool(IDM_CASCADE_LOD_LOW,		LODLow,			*LocalizeUnrealEd("CascadeLODLow"));

	AddSeparator();

	if (!GIsEpicInternal)
	{
		AddTool(IDM_CASCADE_LOD_REGEN,		LODRegenerate,	*LocalizeUnrealEd("CascadeLODRegenerate"));
		AddTool(IDM_CASCADE_LOD_REGENDUP,	LODRegenerateDuplicate,	*LocalizeUnrealEd("CascadeLODRegenerateDuplicate"));
		AddSeparator();
	}
	AddTool(IDM_CASCADE_LOD_DELETE,		LODDelete,		*LocalizeUnrealEd("CascadeLODDelete"));

	Realize();

	if (Cascade && Cascade->EditorOptions)
	{
		if (Cascade->EditorOptions->bShowGrid == TRUE)
		{
			ToggleTool(IDM_CASCADE_TOGGLEGRID, TRUE);
		}
		else
		{
			ToggleTool(IDM_CASCADE_TOGGLEGRID, FALSE);
		}
	}
}

WxCascadeToolBar::~WxCascadeToolBar()
{
}
