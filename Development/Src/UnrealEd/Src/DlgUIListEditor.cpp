/*=============================================================================
DlgUIListEditor.cpp: UI list editor window and support classes
Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "DlgUIListEditor.h"

BEGIN_EVENT_TABLE(WxDlgUIListEditor, wxDialog)
	EVT_BUTTON(wxID_CANCEL, OnCancel)
	EVT_GRID_CMD_LABEL_RIGHT_CLICK(ID_UI_LISTGRID, OnLabelRightClick)
END_EVENT_TABLE()

WxDlgUIListEditor::WxDlgUIListEditor(WxUIEditorBase* InEditor, UUIList* InList)  : 
wxDialog(InEditor, wxID_ANY, *LocalizeUI("DlgUIListEditor_Title"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
FSerializableObject(),
UIEditor(InEditor),
ListWidget(InList),
ListGrid(NULL)
{
	wxSizer* MainSizer = new wxBoxSizer(wxHORIZONTAL);
	{
		// List Grid
		wxSizer *GridSizer = new wxStaticBoxSizer(wxVERTICAL, this, *LocalizeUI("DlgUIListEditor_ListGrid"));
		{
			ListGrid = new wxGrid(this, ID_UI_LISTGRID);
			ListGrid->CreateGrid(1, 1);
			GridSizer->Add(ListGrid,1, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(GridSizer, 1, wxEXPAND | wxALL, 5);

		// OK/Cancel Buttons
		wxSizer *ButtonSizer = new wxBoxSizer(wxVERTICAL);
		{
			wxButton* ButtonOK = new wxButton(this, wxID_OK, *LocalizeUnrealEd("OK"));
			ButtonOK->SetDefault();

			ButtonSizer->Add(ButtonOK, 0, wxEXPAND | wxALL, 5);

			wxButton* ButtonCancel = new wxButton(this, wxID_CANCEL, *LocalizeUnrealEd("Cancel"));
			ButtonSizer->Add(ButtonCancel, 0, wxEXPAND | wxALL, 5);
		}
		MainSizer->Add(ButtonSizer, 0, wxALL, 5);
	}
	SetSizer(MainSizer);
	Layout();

	RefreshGrid();

	FWindowUtil::LoadPosSize(TEXT("DlgUIListEditor"), this, -1, -1, 800,600);
}

WxDlgUIListEditor::~WxDlgUIListEditor()
{
	FWindowUtil::SavePosSize(TEXT("DlgUIListEditor"), this);
}

/**
 * Since this class holds onto an object reference, it needs to be serialized
 * so that the objects aren't GCed out from underneath us.
 *
 * @param	Ar			The archive to serialize with.
 */
void WxDlgUIListEditor::Serialize(FArchive& Ar)
{
	Ar << ListWidget;
}



/** Initializes the grid using the values stored in the list widget. */
void WxDlgUIListEditor::RefreshGrid()
{
	const INT NumColumns = ListWidget->GetTotalColumnCount();
	const INT NumRows = ListWidget->GetTotalRowCount();

	ListGrid->Freeze();
	{
		ListGrid->BeginBatch();
		{
			if(ListGrid->GetNumberCols())
			{
				ListGrid->DeleteCols(0, ListGrid->GetNumberCols());
			}
			
			if(ListGrid->GetNumberRows())
			{
				ListGrid->DeleteRows(0, ListGrid->GetNumberRows());
			}		
			

			ListGrid->ClearGrid();
			ListGrid->InsertCols(0, NumColumns);
			ListGrid->InsertRows(0, NumRows);
			

			// Loop through all columns and set headings to the data tag for that column.
			for(INT ColumnIdx = 0; ColumnIdx < NumColumns; ColumnIdx++)
			{
				FName ListName = ListWidget->GetCellBinding(ColumnIdx);
				ListGrid->SetColLabelValue(ColumnIdx, *ListName.ToString());
			}

			// Autosize columns to fit their headings.
			ListGrid->AutoSizeColumns();
		}
		ListGrid->EndBatch();
	}
	ListGrid->Thaw();
	
}

/** Called when the user cancels the dialog, undos all changes made to the list object since the dialog was spawned. */
void WxDlgUIListEditor::OnCancel(wxCommandEvent& event)
{
	EndModal(wxID_CANCEL);
}

/** Called when the user right clicks on a grid label, displays a context menu. */
void WxDlgUIListEditor::OnLabelRightClick(wxGridEvent& Event)
{
	INT ColumnIdx = Event.GetCol();

	WxUIListBindMenu Menu(UIEditor);
	Menu.Create(ListWidget, ColumnIdx);

	FTrackPopupMenu PopupMenu(UIEditor, &Menu);

	PopupMenu.Show();

	RefreshGrid();
}

