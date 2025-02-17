/*=============================================================================
	Modes.h: Windows to handle the different editor modes and the tools
	         specific to them.

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	WxModeBar.
-----------------------------------------------------------------------------*/

class WxModeBar : public wxPanel
{
public:
	WxModeBar( wxWindow* InParent, wxWindowID InID );
	WxModeBar( wxWindow* InParent, wxWindowID InID, FString InPanelName );
	~WxModeBar();

	INT SavedHeight;				// The height recorded when the bar was initially loaded

	wxPanel* Panel;

	virtual void Refresh();
	virtual void SaveData() {}
	virtual void LoadData() {}

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxModeBarDefault.
-----------------------------------------------------------------------------*/

class WxModeBarDefault : public WxModeBar
{
public:
	WxModeBarDefault( wxWindow* InParent, wxWindowID InID );
	~WxModeBarDefault();

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxModeBarTerrainEdit.
-----------------------------------------------------------------------------*/

class WxModeBarTerrainEdit : public WxModeBar
{
public:
	WxModeBarTerrainEdit( wxWindow* InParent, wxWindowID InID );
	~WxModeBarTerrainEdit();

	virtual void SaveData();
	virtual void LoadData();
	void RefreshLayers();

	void OnToolsSelChange( wxCommandEvent& In );
	void SaveData( wxCommandEvent& In );
	void OnUpdateRadiusMin( wxCommandEvent& In );
	void OnUpdateRadiusMax( wxCommandEvent& In );
	void OnTerrainSelChange( wxCommandEvent& In );

	void OnTerrainImport(wxCommandEvent& In);
	void OnTerrainExport(wxCommandEvent& In);
	void OnTerrainCreate(wxCommandEvent& In);
	void OnTerrainHeightMapClassChange(wxCommandEvent& In);
	void OnTerrainHeightMapOnly(wxCommandEvent& In);

	void UI_PerToolSettings( wxUpdateUIEvent& In );

	void FillTerrainHeightMapClassCombo(wxComboBox* pkComboBox);

	DECLARE_EVENT_TABLE()

protected:
	FString StrExportDirectory;
};

/*-----------------------------------------------------------------------------
	WxModeBarTexture.
-----------------------------------------------------------------------------*/

class WxModeBarTexture : public WxModeBar
{
public:
	WxModeBarTexture( wxWindow* InParent, wxWindowID InID );
	~WxModeBarTexture();

	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
	WxModeBarCoverEdit.
-----------------------------------------------------------------------------*/

class WxModeBarCoverEdit : public WxModeBar
{
public:
	WxModeBarCoverEdit( wxWindow* InParent, wxWindowID InID );
	~WxModeBarCoverEdit();

	void OnAddSlot(wxCommandEvent& In);
	void OnDeleteSelected(wxCommandEvent& In);

	WxMaskedBitmap AddLeftB, AddRightB, DeleteB;

	DECLARE_EVENT_TABLE();
};

