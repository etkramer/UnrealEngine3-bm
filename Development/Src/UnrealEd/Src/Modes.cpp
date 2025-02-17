/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"
#include "Factories.h"
#include "UnTerrain.h"

/*-----------------------------------------------------------------------------
	WxModeBar.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxModeBar, wxPanel )
END_EVENT_TABLE()

WxModeBar::WxModeBar( wxWindow* InParent, wxWindowID InID )
	: wxPanel( InParent, InID )
{
	SavedHeight = -1;
}

WxModeBar::WxModeBar( wxWindow* InParent, wxWindowID InID, FString InPanelName )
	: wxPanel( InParent, InID )
{
	SavedHeight = -1;

	Panel = (wxPanel*)wxXmlResource::Get()->LoadPanel( this, *InPanelName );
	check( Panel != NULL );

	Panel->Fit();
	SetSize( InParent->GetRect().GetWidth(), Panel->GetRect().GetHeight() );
}

WxModeBar::~WxModeBar()
{
}

void WxModeBar::Refresh()
{
	LoadData();
}

/*-----------------------------------------------------------------------------
	WxModeBarDefault.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxModeBarDefault, WxModeBar )
END_EVENT_TABLE()

WxModeBarDefault::WxModeBarDefault( wxWindow* InParent, wxWindowID InID )
	: WxModeBar( InParent, InID, TEXT("ID_PANEL_MODEBARDEFAULT") )
{
	LoadData();
	Show(0);
}

WxModeBarDefault::~WxModeBarDefault()
{
}

/*-----------------------------------------------------------------------------
	WxModeBarTerrainEdit.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxModeBarTerrainEdit, WxModeBar )
END_EVENT_TABLE()

WxModeBarTerrainEdit::WxModeBarTerrainEdit( wxWindow* InParent, wxWindowID InID )
	: WxModeBar( InParent, InID, TEXT("ID_PANEL_MODEBARTERRAINEDIT") )
{
	ADDEVENTHANDLER( XRCID("IDCB_TOOLS"), wxEVT_COMMAND_COMBOBOX_SELECTED, &WxModeBarTerrainEdit::OnToolsSelChange );
	ADDEVENTHANDLER( XRCID("IDCK_PERTOOL"), wxEVT_UPDATE_UI, &WxModeBarTerrainEdit::UI_PerToolSettings );
	ADDEVENTHANDLER( XRCID("IDCK_PERTOOL"), wxEVT_COMMAND_CHECKBOX_CLICKED, &WxModeBarTerrainEdit::SaveData );
	ADDEVENTHANDLER( XRCID("IDEC_RADIUS_MIN"), wxEVT_COMMAND_TEXT_UPDATED, &WxModeBarTerrainEdit::OnUpdateRadiusMin );
	ADDEVENTHANDLER( XRCID("IDEC_RADIUS_MAX"), wxEVT_COMMAND_TEXT_UPDATED, &WxModeBarTerrainEdit::OnUpdateRadiusMax );
	ADDEVENTHANDLER( XRCID("IDEC_RADIUS_MIN"), wxEVT_COMMAND_TEXT_ENTER, &WxModeBarTerrainEdit::SaveData );
	ADDEVENTHANDLER( XRCID("IDEC_RADIUS_MAX"), wxEVT_COMMAND_TEXT_ENTER, &WxModeBarTerrainEdit::SaveData );
	ADDEVENTHANDLER( XRCID("IDCB_STRENGTH"), wxEVT_COMMAND_TEXT_UPDATED, &WxModeBarTerrainEdit::SaveData );
	ADDEVENTHANDLER( XRCID("IDCB_STRENGTH"), wxEVT_COMMAND_COMBOBOX_SELECTED, &WxModeBarTerrainEdit::SaveData );
	ADDEVENTHANDLER( XRCID("IDCB_LAYER"), wxEVT_COMMAND_COMBOBOX_SELECTED, &WxModeBarTerrainEdit::SaveData );
	ADDEVENTHANDLER( XRCID("IDCB_TERRAIN"), wxEVT_COMMAND_COMBOBOX_SELECTED, &WxModeBarTerrainEdit::OnTerrainSelChange );
	
	ADDEVENTHANDLER( XRCID("IDPB_TERRAIN_IMPORT"),			wxEVT_COMMAND_BUTTON_CLICKED,	&WxModeBarTerrainEdit::OnTerrainImport);
	ADDEVENTHANDLER( XRCID("IDPB_TERRAIN_EXPORT"),			wxEVT_COMMAND_BUTTON_CLICKED,	&WxModeBarTerrainEdit::OnTerrainExport);
	ADDEVENTHANDLER( XRCID("IDPB_TERRAIN_CREATE"),			wxEVT_COMMAND_BUTTON_CLICKED,	&WxModeBarTerrainEdit::OnTerrainCreate);
	ADDEVENTHANDLER( XRCID("IDCB_TERRAIN_HEIGHTMAP_CLASS"),	wxEVT_COMMAND_COMBOBOX_SELECTED,&WxModeBarTerrainEdit::OnTerrainHeightMapClassChange);
	ADDEVENTHANDLER( XRCID("IDCKB_TERRAIN_HEIGHTMAP_ONLY"),	wxEVT_COMMAND_CHECKBOX_CLICKED,	&WxModeBarTerrainEdit::OnTerrainHeightMapOnly);

	// Init controls

	FEdMode* mode = GEditorModeTools().FindMode( EM_TerrainEdit );
	wxComboBox* cb = (wxComboBox*)FindWindow( XRCID( "IDCB_TOOLS" ) );
	check( cb != NULL );

	for( INT x = 0 ; x < mode->GetTools().Num() ; ++x )
	{
		cb->Append( *mode->GetTools()(x)->GetName(), mode->GetTools()(x) );
	}
	cb->Select( 0 );
	
	cb = (wxComboBox*)FindWindow( XRCID( "IDCB_STRENGTH" ) );
	check( cb != NULL );

	cb->Append( TEXT("5") );
	cb->Append( TEXT("15") );
	cb->Append( TEXT("25") );
	cb->Append( TEXT("50") );
	cb->Append( TEXT("75") );
	cb->Append( TEXT("100") );
	cb->Append( TEXT("200") );

	// Height map classes
	cb = (wxComboBox*)FindWindow(XRCID("IDCB_TERRAIN_HEIGHTMAP_CLASS"));
	check( cb != NULL );

	FillTerrainHeightMapClassCombo(cb);

	Show(0);
	FLocalizeWindow( this );
}

WxModeBarTerrainEdit::~WxModeBarTerrainEdit()
{
}

void WxModeBarTerrainEdit::OnToolsSelChange( wxCommandEvent& In )
{
	wxComboBox* cb = (wxComboBox*)FindWindow( XRCID( "IDCB_TOOLS" ) );
	check( cb != NULL );
	GEditorModeTools().GetCurrentMode()->SetCurrentTool( (FModeTool*)cb->GetClientData( cb->GetSelection() ) );

	LoadData();
}

void WxModeBarTerrainEdit::SaveData( wxCommandEvent& In )
{
	SaveData();
}

void WxModeBarTerrainEdit::OnUpdateRadiusMin( wxCommandEvent& In )
{
	((FEdModeTerrainEditing*)GEditorModeTools().FindMode( EM_TerrainEdit ))->bPerToolSettings = ((wxCheckBox*)FindWindow( XRCID( "IDCK_PERTOOL"  ) ))->GetValue();

	FTerrainToolSettings* settings = (FTerrainToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();

	settings->RadiusMin = appAtoi( ((wxTextCtrl*)FindWindow( XRCID( "IDEC_RADIUS_MIN" ) ))->GetValue().c_str() );
}

void WxModeBarTerrainEdit::OnUpdateRadiusMax( wxCommandEvent& In )
{
	((FEdModeTerrainEditing*)GEditorModeTools().FindMode( EM_TerrainEdit ))->bPerToolSettings = ((wxCheckBox*)FindWindow( XRCID( "IDCK_PERTOOL"  ) ))->GetValue();

	FTerrainToolSettings* settings = (FTerrainToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();

	settings->RadiusMax = appAtoi( ((wxTextCtrl*)FindWindow( XRCID( "IDEC_RADIUS_MAX" ) ))->GetValue().c_str() );
}

void WxModeBarTerrainEdit::SaveData()
{
	((FEdModeTerrainEditing*)GEditorModeTools().FindMode( EM_TerrainEdit ))->bPerToolSettings = ((wxCheckBox*)FindWindow( XRCID( "IDCK_PERTOOL" ) ))->GetValue();

	FTerrainToolSettings* settings = (FTerrainToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();

	// Misc

	settings->RadiusMin = appAtoi( ((wxTextCtrl*)FindWindow( XRCID( "IDEC_RADIUS_MIN" ) ))->GetValue().c_str() );
	settings->RadiusMax = appAtoi( ((wxTextCtrl*)FindWindow( XRCID( "IDEC_RADIUS_MAX" ) ))->GetValue().c_str() );
	settings->Strength = appAtof( ((wxComboBox*)FindWindow( XRCID( "IDCB_STRENGTH" ) ))->GetValue().c_str() );

	// Layers

	FTerrainLayerId	LayerClientData;

	LayerClientData.Id = ((wxComboBox*)FindWindow( XRCID( "IDCB_LAYER" ) ))->GetClientData( ((wxComboBox*)FindWindow( XRCID( "IDCB_LAYER" ) ))->GetSelection() );

	if(LayerClientData.Type == TLT_HeightMap)
		settings->LayerIndex = INDEX_NONE;
	else if(LayerClientData.Type == TLT_Layer)
	{
		settings->DecoLayer = 0;
		settings->LayerIndex = LayerClientData.Index;
	}
	else
	{
		settings->DecoLayer = 1;
		settings->LayerIndex = LayerClientData.Index;
	}

	// HeightMap exporter
	wxComboBox* pkCombo = (wxComboBox*)FindWindow(XRCID("IDCB_TERRAIN_HEIGHTMAP_CLASS"));
	check( pkCombo != NULL );

	wxString SelString = pkCombo->GetString(pkCombo->GetSelection());
	if (SelString.IsEmpty())
	{
		// This should NOT happen...
		return;
	}
	settings->HeightMapExporterClass = FString(TEXT("TerrainHeightMapExporter")) + FString(SelString.c_str());
}

void WxModeBarTerrainEdit::LoadData()
{
	FTerrainToolSettings* settings = (FTerrainToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();

	// Misc

	((wxTextCtrl*)FindWindow( XRCID( "IDEC_RADIUS_MIN" ) ) )->SetValue( *FString::Printf( TEXT("%d"), settings->RadiusMin ) );
	((wxTextCtrl*)FindWindow( XRCID( "IDEC_RADIUS_MAX" ) ) )->SetValue( *FString::Printf( TEXT("%d"), settings->RadiusMax ) );
	((wxComboBox*)FindWindow( XRCID( "IDCB_STRENGTH" ) ) )->SetValue( *FString::Printf( TEXT("%1.2f"), settings->Strength ) );

	((wxCheckBox*)FindWindow( XRCID( "IDCK_PERTOOL" ) ) )->SetValue( ((FEdModeTerrainEditing*)GEditorModeTools().FindMode( EM_TerrainEdit ))->bPerToolSettings == TRUE );

	// Terrains
	wxComboBox* Wk = (wxComboBox*)FindWindow( XRCID( "IDCB_TERRAIN" ) );
	check( Wk != NULL );

	INT save = Wk->GetSelection();
	if( save == -1 )
	{
		save = 0;
	}

	Wk->Clear();
	for( FActorIterator It; It; ++It )
	{
		ATerrain* Terrain = Cast<ATerrain>(*It);
		if( Terrain )
		{
			Wk->Append( *Terrain->GetName(), Terrain );
		}
	}
	Wk->SetSelection(save);

	wxCommandEvent DummyEvent;
	OnTerrainSelChange( DummyEvent );

	// HeightMap exporter
	Wk = (wxComboBox*)FindWindow(XRCID("IDCB_TERRAIN_HEIGHTMAP_CLASS"));
	check( Wk != NULL );

	FillTerrainHeightMapClassCombo(Wk);
}

// Load the layer combobox based on the currently selected terraininfo

void WxModeBarTerrainEdit::RefreshLayers()
{
	FEdModeTerrainEditing* mode = (FEdModeTerrainEditing*)GEditorModeTools().FindMode( EM_TerrainEdit );
	wxComboBox* cb = (wxComboBox*)FindWindow( XRCID( "IDCB_LAYER" ) );
	check( cb != NULL );

	cb->Clear();
	cb->Append( *LocalizeUnrealEd("Heightmap"), (void*)NULL );
	cb->Append( TEXT("---") );

	if( mode->CurrentTerrain )
	{
		for( INT x = 1 ; x < mode->CurrentTerrain->Layers.Num() ; ++x )
		{
			FTerrainLayerId	LayerClientData;
			LayerClientData.Type = TLT_Layer;
			LayerClientData.Index = x;
			cb->Append( *mode->CurrentTerrain->Layers(x).Name, LayerClientData.Id );
		}

		for( INT x = 0 ; x < mode->CurrentTerrain->DecoLayers.Num() ; ++x )
		{
			FTerrainLayerId	LayerClientData;
			LayerClientData.Type = TLT_DecoLayer;
			LayerClientData.Index = x;
			cb->Append( *mode->CurrentTerrain->DecoLayers(x).Name, LayerClientData.Id );
		}
	}
}

void WxModeBarTerrainEdit::OnTerrainSelChange( wxCommandEvent& In )
{
	FEdModeTerrainEditing* mode = (FEdModeTerrainEditing*)GEditorModeTools().FindMode( EM_TerrainEdit );
	wxComboBox* cb = (wxComboBox*)FindWindow( XRCID( "IDCB_TERRAIN" ) );
	check( cb != NULL );

	if( cb->GetSelection() != -1 )
	{
		mode->CurrentTerrain = (ATerrain*)cb->GetClientData( cb->GetSelection() );
	}

	RefreshLayers();

	// Select the heightmap

	((wxComboBox*)FindWindow( XRCID( "IDCB_LAYER" ) ) )->SetSelection( 0 );
}

void WxModeBarTerrainEdit::OnTerrainImport(wxCommandEvent& In)
{
	wxString	Message(TEXT("Export to..."));
	wxString	DefaultDir(*StrExportDirectory);
	wxString	DefaultFile(TEXT(""));
	wxString	Wildcard;

	FString		ImportFile;

	const UBOOL bHeightMapOnly = (UBOOL)((wxCheckBox*)FindWindow(XRCID("IDCKB_TERRAIN_HEIGHTMAP_ONLY")))->GetValue();
	if (bHeightMapOnly)
	{
		debugf(TEXT("TerrainImport: HeightMap only!"));

		Wildcard = wxString(TEXT("BMP files (*.bmp)|*.bmp|All files (*.*)|*.*"));
	}
	else
	{
		Wildcard = wxString(TEXT("T3D files (*.T3D)|*.T3D"));
	}


	WxFileDialog ImportFileDlg(GApp->EditorFrame, Message, DefaultDir, DefaultFile, Wildcard, wxOPEN);
	if (ImportFileDlg.ShowModal() != wxID_OK)
	{
		return;
	}

	ImportFile = FString(ImportFileDlg.GetPath());
	debugf(TEXT("TerrainEdit Importing %s"), *ImportFile);

	if (bHeightMapOnly)
	{
		// The extension of the file, will indicate the importer...
		FFilename FileName(*ImportFile);

		if (FileName.GetExtension() == TEXT("BMP"))
		{
			AActor* Actor = GWorld->SpawnActor(ATerrain::StaticClass(), NAME_None, FVector(0,0,0), FRotator(0,0,0), NULL, 1, 0);
			check(Actor);

			ATerrain* Terrain = Cast<ATerrain>(Actor);

			UTerrainHeightMapFactoryG16BMP* pkFactory = ConstructObject<UTerrainHeightMapFactoryG16BMP>(UTerrainHeightMapFactoryG16BMP::StaticClass());
			check(pkFactory);

			if (pkFactory->ImportHeightDataFromFile(Terrain, *FileName, GWarn, true) == false)
			{
			}
		}
	}
}

void WxModeBarTerrainEdit::OnTerrainExport(wxCommandEvent& In)
{
	TArray<ATerrain*> SelectedTerrain;

	// If there is no terrain selected, don't do anything.
	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		ATerrain* TerrainActor = Cast<ATerrain>( Actor );
		if ( TerrainActor )
		{
			SelectedTerrain.AddItem( TerrainActor );
		}
	}

	if (SelectedTerrain.Num() == 0)
	{
		appMsgf(AMT_OK, TEXT("No terrain selected"));
		return;
	}

	const UBOOL bHeightMapOnly = (UBOOL)((wxCheckBox*)FindWindow(XRCID("IDCKB_TERRAIN_HEIGHTMAP_ONLY")))->GetValue();
	if (bHeightMapOnly)
	{
		debugf(TEXT("TerrainExport: HeightMap only!"));

		// Get the HeightMapExporter class.
		FString HMExporter;

		if (!GConfig->GetString(TEXT("Editor.EditorEngine"), TEXT("HeightMapExportClassName"), HMExporter, GEditorIni))
		{
			HMExporter = TEXT("TerrainHeightMapExporterTextT3D");
			GConfig->SetString(TEXT("Editor.EditorEngine"), TEXT("HeightMapExportClassName"), *HMExporter, GEditorIni);
		}

		UTerrainHeightMapExporter* pkExporter = NULL;
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (It->IsChildOf(UTerrainHeightMapExporter::StaticClass()))
			{
				if (appStrcmp(&(**It->GetName()), *HMExporter) == 0)
				{
					pkExporter = ConstructObject<UTerrainHeightMapExporter>(*It);
				}
			}
		}

		if (pkExporter == NULL)
		{
			appMsgf(AMT_OK, TEXT("Could not find HeightMap exporter"));
			return;
		}

		//@todo. Store the MRU directory???
		// Get the directory to export to
		FName	DialogTitle(TEXT("Export to..."));
		FName	DefaultDir(*StrExportDirectory);
		if ( PromptUserForDirectory(StrExportDirectory, DialogTitle, DefaultDir) )
		{
			for (INT ii = 0; ii < SelectedTerrain.Num(); ii++)
			{
				ATerrain* Terrain = SelectedTerrain(ii);
				if (!pkExporter->ExportHeightDataToFile(Terrain, *StrExportDirectory, GWarn))
				{
					appMsgf(AMT_OK, TEXT("Failed to export HeightMap for terrain %s"), *Terrain->GetName());
				}
			}
		}
	}
	else
	{
		debugf(TEXT("TerrainExport: Complete!"));
	}
}

void WxModeBarTerrainEdit::OnTerrainCreate(wxCommandEvent& In)
{
}

void WxModeBarTerrainEdit::OnTerrainHeightMapClassChange(wxCommandEvent& In)
{
	// Grab the selection
	wxComboBox* pkCombo = (wxComboBox*)FindWindow(XRCID("IDCB_TERRAIN_HEIGHTMAP_CLASS"));
	check( pkCombo != NULL );

	wxString SelString = pkCombo->GetString(pkCombo->GetSelection());
	if (SelString.IsEmpty())
	{
		// This should NOT happen...
		return;
	}

	// Add on the "TerrainHeightMapExporter"
	FString strSelected = FString(TEXT("TerrainHeightMapExporter"));
	strSelected += FString(SelString.c_str());
	GConfig->SetString(TEXT("Editor.EditorEngine"), TEXT("HeightMapExportClassName"), 
		*strSelected, GEditorIni);
}

void WxModeBarTerrainEdit::OnTerrainHeightMapOnly(wxCommandEvent& In)
{
}

void WxModeBarTerrainEdit::UI_PerToolSettings( wxUpdateUIEvent& In )
{
	In.Check( ((FEdModeTerrainEditing*)GEditorModeTools().FindMode( EM_TerrainEdit ))->bPerToolSettings == TRUE );
}

void WxModeBarTerrainEdit::FillTerrainHeightMapClassCombo(wxComboBox* pkComboBox)
{
	pkComboBox->Clear();

	FString SelectedString;
	GConfig->GetString(TEXT("Editor.EditorEngine"), TEXT("HeightMapExportClassName"), 
		SelectedString, GEditorIni);

	FTerrainToolSettings* settings = 
		(FTerrainToolSettings*)GEditorModeTools().GetCurrentMode()->GetSettings();
	if (settings)
	{
		if (appStrlen(*(settings->HeightMapExporterClass)) == 0)
		{
			settings->HeightMapExporterClass = SelectedString;
		}
	}

	INT iDefLen = appStrlen(TEXT("TerrainHeightMapExporter"));

	INT iSelected = -1;
	INT iCount = -1;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(UTerrainHeightMapExporter::StaticClass()))
		{
			UBOOL bSelected = false;
			// Get the name
			FString strName = It->GetFName().ToString();
			if (appStrlen(*strName) - iDefLen)
			{
				if (strName == SelectedString)
				{
					bSelected = true;
				}

				// 'Parse' off the "TerrainHeightMapExporter" portion
				FString strParsed = strName.Right(appStrlen(*strName) - iDefLen);
				// If it isn't the 'base' exporter, put it in the list.
				if (strParsed != TEXT(""))
				{
					iCount++;
					if (bSelected)
					{
						iSelected = iCount;
					}
					pkComboBox->Append(*strParsed);
				}
			}
		}
	}

	if (iSelected != -1)
	{
		pkComboBox->SetSelection(iSelected);
	}
}

/*-----------------------------------------------------------------------------
	WxModeBarTexture.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxModeBarTexture, WxModeBar )
END_EVENT_TABLE()

WxModeBarTexture::WxModeBarTexture( wxWindow* InParent, wxWindowID InID )
	: WxModeBar( InParent, InID, TEXT("ID_PANEL_MODEBARTEXTURE") )
{
	LoadData();
	Show(0);
}

WxModeBarTexture::~WxModeBarTexture()
{
}

/*-----------------------------------------------------------------------------
	WxModeBarCoverEdit.
-----------------------------------------------------------------------------*/

BEGIN_EVENT_TABLE( WxModeBarCoverEdit, WxModeBar )
	EVT_TOOL(IDM_COVEREDIT_ADDSLOT, WxModeBarCoverEdit::OnAddSlot)
	EVT_TOOL(IDM_COVEREDIT_DELETESLOT, WxModeBarCoverEdit::OnDeleteSelected)
END_EVENT_TABLE()

WxModeBarCoverEdit::WxModeBarCoverEdit( wxWindow* InParent, wxWindowID InID )
	: WxModeBar( InParent, InID, TEXT("ID_PANEL_MODEBARCOVEREDIT") )
{
	/*
	AddLeftB.Load(TEXT("KIS_Parent"));
	AddRightB.Load(TEXT("KIS_Rename"));
	DeleteB.Load(TEXT("KIS_Search"));

	SetToolBitmapSize( wxSize( 18, 18 ) );

	AddSeparator();
	AddTool(IDM_COVEREDIT_ADDSLOT, AddLeftB, TEXT("Add Slot"));
	AddSeparator();
	AddTool(IDM_COVEREDIT_DELETESLOT, DeleteB, TEXT("Delete Slot"));
	*/
}

WxModeBarCoverEdit::~WxModeBarCoverEdit()
{
}

void WxModeBarCoverEdit::OnAddSlot(wxCommandEvent &In)
{
	ACoverLink *Link = GEditor->GetSelectedActors()->GetTop<ACoverLink>();
	if (Link != NULL)
	{
		FVector Offset(0.f,-128.f,0.f);
		TArray<FCoverSlot> &Slots = Link->Slots;
		INT Idx = Slots.AddZeroed();
		// if other slots are in this list
		if (Idx >= 1)
		{
			// use previous slot's location plus a slight offset
			Slots(Idx).LocationOffset = Slots(Idx-1).LocationOffset + Offset;
			// use previous slot's rotation
			Slots(Idx).RotationOffset = Slots(Idx-1).RotationOffset;
		}
		else
		{
			// otherwise use the base offset
			Slots(Idx).LocationOffset = Offset;
		}
	}
}

void WxModeBarCoverEdit::OnDeleteSelected(wxCommandEvent &In)
{
	ACoverLink *Link = GEditor->GetSelectedActors()->GetTop<ACoverLink>();
	if (Link != NULL)
	{
		// check to see if a slot is selected
		UBOOL bFoundSlot = 0;
		// check left first
		for (INT Idx = 0; Idx < Link->Slots.Num() && !bFoundSlot; Idx++)
		{
			if (Link->Slots(Idx).bSelected)
			{
				// remove the slot
				Link->Slots.Remove(Idx--,1);
				bFoundSlot = 1;
			}
		}
		/*
		// if no slot selected
		if (!bFoundSlot)
		{
			// then delete the actual link
			GEditor->Exec(TEXT("DELETE"));
		}
		*/
	}
}

