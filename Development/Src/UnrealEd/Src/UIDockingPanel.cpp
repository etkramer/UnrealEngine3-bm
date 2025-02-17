/*=============================================================================
	UIDockingPanel.cpp: Wx Panel that lets the user change docking settings for the currently selected widget. 
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"

#include "EngineUIPrivateClasses.h"
#include "UnrealEdPrivateClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "UnObjectEditor.h"
#include "UnUIEditor.h"
#include "UIDockingPanel.h"
#include "ScopedTransaction.h"
#include "ScopedObjectStateChange.h"

BEGIN_EVENT_TABLE(WxUIDockingSetEditorPanel,wxPanel)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_LEFT_TARGET,WxUIDockingSetEditorPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_TOP_TARGET,WxUIDockingSetEditorPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_RIGHT_TARGET,WxUIDockingSetEditorPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_BOTTOM_TARGET,WxUIDockingSetEditorPanel::OnChangeDockingTarget)

	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_LEFT_FACE,WxUIDockingSetEditorPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_TOP_FACE,WxUIDockingSetEditorPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_RIGHT_FACE,WxUIDockingSetEditorPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_BOTTOM_FACE,WxUIDockingSetEditorPanel::OnChangeDockingFace)

	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_LEFT,WxUIDockingSetEditorPanel::OnChangeDockingEvalType)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_TOP,WxUIDockingSetEditorPanel::OnChangeDockingEvalType)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_RIGHT,WxUIDockingSetEditorPanel::OnChangeDockingEvalType)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_BOTTOM,WxUIDockingSetEditorPanel::OnChangeDockingEvalType)
END_EVENT_TABLE()

void WxUIDockingSetEditorPanel::Serialize( FArchive& Ar )
{
	Ar << CurrentWidget << ValidDockTargets;
}


/**
 * Creates the controls for this window
 */
WxUIDockingSetEditorPanel::WxUIDockingSetEditorPanel( wxWindow* InParent )
: wxPanel(InParent)
{
	wxBoxSizer* DockingSetVSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(DockingSetVSizer);

	// retrieve the text for the docking set labels from the loc file
	FString tmpString = LocalizeUI( TEXT("DlgDockingEditor_Label_DockTarget") );
	tmpString.ParseIntoArray(&DockFaceStrings,TEXT(","),TRUE);
	if ( DockFaceStrings.Num() != 4 )
	{
		DockFaceStrings.Empty(4);
		DockFaceStrings.AddItem(LocalizeUI("UIEditor_FaceText[0]"));
		DockFaceStrings.AddItem(LocalizeUI("UIEditor_FaceText[1]"));
		DockFaceStrings.AddItem(LocalizeUI("UIEditor_FaceText[2]"));
		DockFaceStrings.AddItem(LocalizeUI("UIEditor_FaceText[3]"));
	}
	FString NoneText = *LocalizeUnrealEd(TEXT("None"));
	DockFaceStrings.AddItem(NoneText);

	EvalTypeStrings.Empty(UIPADDINGEVAL_MAX);
	for ( INT EvalIndex = 0; EvalIndex < UIPADDINGEVAL_MAX; EvalIndex++ )
	{
		EvalTypeStrings.AddItem(LocalizeUI(*FString::Printf(TEXT("UIEditor_ExtentEvalType[%i]"), EvalIndex)));
	}

	FString DockTargetToolTip = LocalizeUI(TEXT("DlgDockingEditor_ToolTip_DockTarget"));
	FString DockFaceToolTip = LocalizeUI(TEXT("DlgDockingEditor_ToolTip_DockFace"));
	FString DockPaddingToolTip = LocalizeUI(TEXT("DlgDockingEditor_ToolTip_DockPadding"));
	FString DockPaddingTypeToolTip = LocalizeUI(TEXT("DlgDockingEditor_ToolTip_DockPaddingType"));
	FString DockTargetHelpText = LocalizeUI(TEXT("DlgDockingEditor_HelpText_DockTarget"));
	FString DockFaceHelpText = LocalizeUI(TEXT("DlgDockingEditor_HelpText_DockFace"));
	FString DockPaddingHelpText = LocalizeUI(TEXT("DlgDockingEditor_HelpText_DockPadding"));
	FString DockPaddingTypeHelpText = LocalizeUI(TEXT("DlgDockingEditor_HelpText_DockPaddingType"));

	// add the controls for each face
	for ( INT i = 0; i < UIFACE_MAX; i++ )
	{
		wxBoxSizer* DockFaceVSizer = new wxBoxSizer(wxVERTICAL);
		DockingSetVSizer->Add(DockFaceVSizer, 0, wxGROW|wxALL, 0);

		wxStaticBox* DockFaceStaticBox = new wxStaticBox(this, wxID_ANY, *(DockFaceStrings(i) + TEXT(":")));
		wxStaticBoxSizer* DockFaceStaticHSizer = new wxStaticBoxSizer(DockFaceStaticBox, wxHORIZONTAL);
		DockFaceVSizer->Add(DockFaceStaticHSizer, 0, wxGROW|wxALL, 0);

		wxBoxSizer* DockFaceComboSizer = new wxBoxSizer(wxVERTICAL);
		DockFaceStaticHSizer->Add(DockFaceComboSizer, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		{
			// add the combo that displays the list of widgets that can be assigned as a dock target
			cmb_DockTarget[i] = new wxComboBox( this, ID_UI_DOCKINGEDITOR_LEFT_TARGET + i, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
			cmb_DockTarget[i]->SetHelpText(*DockTargetHelpText);
			cmb_DockTarget[i]->SetToolTip(*DockTargetToolTip);
			DockFaceComboSizer->Add(cmb_DockTarget[i], 0, wxGROW|wxBOTTOM, 5);

			wxArrayString cmb_PaddingType_Strings;
			for ( INT Idx = 0; Idx < EvalTypeStrings.Num(); Idx++ )
			{
				cmb_PaddingType_Strings.Add(*EvalTypeStrings(Idx));
			}
			cmb_PaddingEvalType[i] = new wxComboBox( this, ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_LEFT+i, wxEmptyString, wxDefaultPosition, wxDefaultSize, cmb_PaddingType_Strings, wxCB_READONLY );
			cmb_PaddingEvalType[i]->SetHelpText(*DockPaddingTypeHelpText);
			cmb_PaddingEvalType[i]->SetToolTip(*DockPaddingTypeToolTip);
			DockFaceComboSizer->Add(cmb_PaddingEvalType[i], 0, wxGROW|wxALL, 0);
		}

		wxBoxSizer* DockFaceValueSizer = new wxBoxSizer(wxVERTICAL);
		DockFaceStaticHSizer->Add(DockFaceValueSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		{
			wxString cmb_DockFace_Strings[] =
			{
				*DockFaceStrings(0),
				*DockFaceStrings(1),
				*DockFaceStrings(2),
				*DockFaceStrings(3),
				*DockFaceStrings(4)
			};
			// add the combo that displays the list of faces that can be assigned to this docking set
			cmb_DockFace[i] = new wxComboBox( this, ID_UI_DOCKINGEDITOR_LEFT_FACE + i, wxEmptyString, wxDefaultPosition, wxDefaultSize, 5, cmb_DockFace_Strings, wxCB_READONLY );
			cmb_DockFace[i]->SetValue(cmb_DockTarget[i]->GetString(0));
			cmb_DockFace[i]->SetHelpText(*DockFaceHelpText);
			cmb_DockFace[i]->SetToolTip(*DockFaceToolTip);
			DockFaceValueSizer->Add(cmb_DockFace[i], 0, wxGROW|wxBOTTOM, 5);

			// add a spin control that allows the user to modify the padding value for this face
			spin_DockPadding[i] = new WxSpinCtrlReal(this, ID_UI_DOCKINGEDITOR_LEFT_PADDING + i);
			spin_DockPadding[i]->SetHelpText(*DockPaddingHelpText);
			spin_DockPadding[i]->SetToolTip(*DockPaddingToolTip);
			DockFaceValueSizer->Add( spin_DockPadding[i], 0, wxGROW|wxALL, 0);
		}
	}
}


/**
 * Sets the widget that this panel will edit docking sets for, and refreshes all control values with the specified widget's docking set values.
 */
void WxUIDockingSetEditorPanel::RefreshEditorPanel( UUIObject* InWidget )
{
	CurrentWidget = InWidget;
	if ( CurrentWidget != NULL )
	{
		// the widget can never dock to itself, so remove it from the list of valid targets right away
		ValidDockTargets = InWidget->GetScene()->GetChildren(TRUE);
		ValidDockTargets.RemoveItem(CurrentWidget);

		// fill the controls with this widget's data
		InitializeTargetCombos();
	}
	else
	{
		ValidDockTargets.Empty();

		// clear the control values
		for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
		{
			// slight hack here because SetSelection doesn't update the combo's m_selectionOld member, which causes
			// the combo to generate two "selection changed" events the first time the user selects an item
			wxString SelectedString = cmb_DockTarget[FaceIndex]->GetString(0);
			cmb_DockTarget[FaceIndex]->SetValue(SelectedString);

			SelectedString = cmb_DockFace[FaceIndex]->GetString(0);
			cmb_DockFace[FaceIndex]->SetValue(SelectedString);

			SelectedString = cmb_PaddingEvalType[FaceIndex]->GetString(0);
			cmb_PaddingEvalType[FaceIndex]->SetValue(SelectedString);

			spin_DockPadding[FaceIndex]->SetValue(0.0f);
		}
	}
}


/**
 * Fills the "dock target" combos with the names of the widgets within the owner container that are valid dock targets for CurrentWidget.
 */
void WxUIDockingSetEditorPanel::InitializeTargetCombos()
{
	if ( CurrentWidget != NULL )
	{
		Sort<USE_COMPARE_POINTER(UUIScreenObject,UnUIEditor_AlphaSort)>( (UUIScreenObject**)&ValidDockTargets(0), ValidDockTargets.Num() );

		UUIScene* OwnerScene = CurrentWidget->GetScene();
		for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
		{
			EUIWidgetFace CurrentFace = (EUIWidgetFace)FaceIndex;
			cmb_DockTarget[CurrentFace]->Freeze();

			// clear the existing contents
			cmb_DockTarget[CurrentFace]->Clear();

			// Add the 'none' item
			INT SelectionIndex=0;
			cmb_DockTarget[CurrentFace]->Append(*LocalizeUnrealEd(TEXT("None")), (void*)NULL);

			// add the item for the scene
			FString SceneTag = FString::Printf(TEXT("%s (%s)"), *OwnerScene->GetTag().ToString(), *LocalizeUI("UIEditor_Scene"));
			INT idx = cmb_DockTarget[FaceIndex]->Append(*SceneTag, OwnerScene);
			if ( CurrentWidget->DockTargets.IsDocked(CurrentFace,FALSE) && CurrentWidget->DockTargets.GetDockTarget(CurrentFace) == NULL )
			{
				// if the widget is docked, but GetDockTarget returns NULL, the widget is docked to the scene
				SelectionIndex = idx;
			}

			for ( INT ChildIndex = 0; ChildIndex < ValidDockTargets.Num(); ChildIndex++ )
			{
				UUIObject* Child = ValidDockTargets(ChildIndex);

				UBOOL bCurrentlyDockedToChild = CurrentWidget->DockTargets.IsDocked(CurrentFace, TRUE)
											&& (((UUIScreenObject*)OwnerScene) == Child || CurrentWidget->DockTargets.GetDockTarget(CurrentFace) == Child);
				UBOOL bIsValidTarget = bCurrentlyDockedToChild;
				if ( !bIsValidTarget )
				{
					for ( INT Idx = 0; Idx < UIFACE_MAX; Idx++ )
					{
						if ( CurrentWidget->IsValidDockTarget(CurrentFace, Child, static_cast<EUIWidgetFace>(Idx)) )
						{
							bIsValidTarget = TRUE;
							break;
						}
					}
				}

				if ( bIsValidTarget )
				{
					idx = cmb_DockTarget[CurrentFace]->Append( *Child->GetTag().ToString(), Child );
					if ( bCurrentlyDockedToChild || CurrentWidget->DockTargets.GetDockTarget(CurrentFace) == Child )
					{
						SelectionIndex = idx;
					}
				}
			}

			cmb_DockTarget[CurrentFace]->Thaw();

			// slight hack here because SetSelection doesn't update the combo's m_selectionOld member, which causes
			// the combo to generate two "selection changed" events the first time the user selects an item
			//cmb_DockTarget[FaceIndex]->SetSelection(SelectionIndex);
			wxString SelectedWidgetString = cmb_DockTarget[FaceIndex]->GetString(SelectionIndex);
			cmb_DockTarget[FaceIndex]->SetValue(SelectedWidgetString);

			// set the value of the face combo
			cmb_DockFace[FaceIndex]->SetValue(*DockFaceStrings(CurrentWidget->DockTargets.GetDockFace(CurrentFace)));

			// set the value of the padding eval type combo
			EUIDockPaddingEvalType EvalType = CurrentWidget->DockTargets.GetDockPaddingType(CurrentFace);
			if ( EvalType == UIPADDINGEVAL_MAX )
			{
				EvalType = UIPADDINGEVAL_Pixels;
			}


			PreviousPaddingType[FaceIndex] = EvalType;
			cmb_PaddingEvalType[FaceIndex]->SetValue(*EvalTypeStrings(EvalType));

			// set the value of the padding control
			if ( EvalType == UIPADDINGEVAL_Pixels )
			{
				spin_DockPadding[FaceIndex]->SetMinValue(-100000.0f, FALSE);
				spin_DockPadding[FaceIndex]->SetMaxValue(100000.0f, FALSE);
				spin_DockPadding[FaceIndex]->SetFixedIncrementAmount(1.0f);
			}
			else
			{
				spin_DockPadding[FaceIndex]->SetMinValue(-100.0f, FALSE);
				spin_DockPadding[FaceIndex]->SetMaxValue(100.0f, FALSE);
				spin_DockPadding[FaceIndex]->SetFixedIncrementAmount(0.1f);
			}
			spin_DockPadding[FaceIndex]->SetValue(CurrentWidget->DockTargets.GetDockPadding(CurrentFace, EvalType));
		}

		for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
		{
			UpdateAvailableDockFaces((EUIWidgetFace)FaceIndex);
		}
	}
}


/**
 * Refreshes and validates the list of faces which can be used as the dock target face for the specified face.  Removes any entries that
 * are invalid for this face (generally caused by restrictions on docking relationships necessary to prevent infinite recursion)
 *
 * @param	SourceFace	indicates which of the combo boxes should be validated & updated
 */
void WxUIDockingSetEditorPanel::UpdateAvailableDockFaces( EUIWidgetFace SourceFace )
{
	UUIScreenObject* CurrentSelectedTarget = GetSelectedDockTarget(SourceFace);
	UUIObject* CurrentlySelectedWidget = Cast<UUIObject>(CurrentSelectedTarget);

	TArray<FString> ValidDockFaces = DockFaceStrings;
	if ( CurrentSelectedTarget != NULL )
	{
		for ( INT i = 0; i < UIFACE_MAX; i++ )
		{
			EUIWidgetFace CurrentFace = (EUIWidgetFace)i;
			if ( CurrentlySelectedWidget != NULL )
			{
				if ( !CurrentWidget->IsValidDockTarget(SourceFace, CurrentlySelectedWidget, CurrentFace) )
				{
					ValidDockFaces.RemoveItem(DockFaceStrings(CurrentFace));
				}
			}

			UUIScreenObject* TargetObject = GetSelectedDockTarget(CurrentFace);
			EUIWidgetFace TargetFace = GetSelectedDockFace(CurrentFace);
			if ( TargetFace != UIFACE_MAX )
			{
				// this the target for SourceFace is also the target for another face,
				if ( TargetObject == CurrentSelectedTarget && ((CurrentFace % UIORIENT_MAX) != (TargetFace % UIORIENT_MAX)) )
				{
					// not allowed to bind more than one face of source widget to the same face of the target widget
					ValidDockFaces.RemoveItem(DockFaceStrings(TargetFace));
				}
			}
		}
	}

	// save the current selection
	FString CurrentSelectedFaceString;
	if ( cmb_DockFace[SourceFace]->GetCount() > 0 )
	{
		CurrentSelectedFaceString = cmb_DockFace[SourceFace]->GetValue().c_str();
	}

	// get the index for the currently selected face
	INT CurrentSelectedFaceIndex = CurrentSelectedFaceString.Len()
		? ValidDockFaces.FindItemIndex(CurrentSelectedFaceString)
		: INDEX_NONE;

	if ( CurrentSelectedFaceIndex == INDEX_NONE )
	{
		// if the selected face is no longer part of the valid set, change it to None
		CurrentSelectedFaceIndex = ValidDockFaces.Num() - 1;
	}

	// freeze the combo to eliminate flicker while we update the combo's items
	cmb_DockFace[SourceFace]->Freeze();

	// now clear the existing entries
	cmb_DockFace[SourceFace]->Clear();
	for ( INT i = 0; i < ValidDockFaces.Num(); i++ )
	{
		// add the remaining choices
		cmb_DockFace[SourceFace]->Append(*ValidDockFaces(i)); 
	}

	cmb_DockFace[SourceFace]->Thaw();

	// restore the selected item
	cmb_DockFace[SourceFace]->SetValue(*ValidDockFaces(CurrentSelectedFaceIndex));
}

/**
 * Returns the widget corresponding to the selected item in the "target widget" combo box for the specified face
 *
 * @param	SourceFace	indicates which of the "target widget" combo boxes to retrieve the value from
 */
UUIScreenObject* WxUIDockingSetEditorPanel::GetSelectedDockTarget( EUIWidgetFace SourceFace )
{
	UUIScreenObject* Result = NULL;
	if ( cmb_DockTarget[SourceFace]->GetCount() > 0 )
	{
		Result = (UUIScreenObject*)cmb_DockTarget[SourceFace]->GetClientData(cmb_DockTarget[SourceFace]->GetSelection());
	}
	return Result;
}

/**
 * Returns the UIWidgetFace corresponding to the selected item in the "target face" combo box for the specified face
 *
 * @param	SourceFace	indicates which of the "target face" combo boxes to retrieve the value from
 */
EUIWidgetFace WxUIDockingSetEditorPanel::GetSelectedDockFace( EUIWidgetFace SourceFace )
{
	wxString Selection;
	if ( cmb_DockFace[SourceFace]->GetCount() > 0 )
	{
		Selection = cmb_DockFace[SourceFace]->GetValue();
	}
	INT Index = Selection.Len() > 0
		? DockFaceStrings.FindItemIndex(Selection.c_str())
		: UIFACE_MAX;

	EUIWidgetFace Result = (EUIWidgetFace)Index;
	return Result;
}

/**
 * Gets the padding value from the "padding" spin control of the specified face
 *
 * @param	SourceFace	indicates which of the "padding" spin controls to retrieve the value from
 */
FLOAT WxUIDockingSetEditorPanel::GetSelectedDockPadding( EUIWidgetFace SourceFace )
{
	FLOAT Result = spin_DockPadding[SourceFace]->GetValue();
	return Result;
}


/**
 * Gets the evaluation type for the padding of the specified face
 *
 * @param	SourceFace	indicates which face to retrieve
 */
EUIDockPaddingEvalType WxUIDockingSetEditorPanel::GetSelectedDockPaddingType( EUIWidgetFace SourceFace )
{
	checkSlow(SourceFace<UIFACE_MAX);

	wxString Selection;
	if ( cmb_PaddingEvalType[SourceFace]->GetCount() > 0 )
	{
		Selection = cmb_PaddingEvalType[SourceFace]->GetValue();
	}

	INT Index = Selection.Len() > 0
		? EvalTypeStrings.FindItemIndex(Selection.c_str())
		: UIPADDINGEVAL_Pixels;

	if ( Index >= UIPADDINGEVAL_MAX )
	{
		Index = UIPADDINGEVAL_Pixels;
	}
	EUIDockPaddingEvalType Result = static_cast<EUIDockPaddingEvalType>(Index);
	return Result;
}

/**
 * Called when the user changes the value of a "target widget" combo box.  Refreshes and validates the 
 * choices available in the "target face" combo for that source face.
 */
void WxUIDockingSetEditorPanel::OnChangeDockingTarget( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_TARGET);
	UpdateAvailableDockFaces(SourceFace);

	// pass to our owner control
	Event.Skip();
}

/**
 * Called when the user changes the value of a "target face" combo box.  Refreshes and validates the
 * choices available in the "target face" combo for all OTHER source faces.
 */
void WxUIDockingSetEditorPanel::OnChangeDockingFace( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_FACE);
	for ( INT i = 0; i < UIFACE_MAX; i++ )
	{
		if ( i != SourceFace )
		{
			UpdateAvailableDockFaces((EUIWidgetFace)i);
		}
	}

	// pass to our owner control
	Event.Skip();
}

/**
 * Called when the user changes the scale type for a dock padding.  Converts the existing dock padding value into
 * the new scale type and updates the min/max values for the dock padding value spin control.
 */
void WxUIDockingSetEditorPanel::OnChangeDockingEvalType( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_LEFT);
	
	// convert the current value into the new scale type
	FUIScreenValue_DockPadding TempPadding(EC_EventParm);
	TempPadding.ChangePaddingScaleType(CurrentWidget, SourceFace, PreviousPaddingType[SourceFace], FALSE);
	TempPadding.SetPaddingValue(CurrentWidget, GetSelectedDockPadding(SourceFace), SourceFace, PreviousPaddingType[SourceFace], FALSE);

	EUIDockPaddingEvalType NewPaddingType = GetSelectedDockPaddingType(SourceFace);
	TempPadding.ChangePaddingScaleType(CurrentWidget, SourceFace, NewPaddingType, TRUE);

	const FLOAT ConvertedValue = TempPadding.GetPaddingValue(CurrentWidget, SourceFace, NewPaddingType);

	// update the min/max values for the dock padding value spin control
	if ( NewPaddingType == UIPADDINGEVAL_Pixels )
	{
		spin_DockPadding[SourceFace]->SetMinValue(-100000.0f, FALSE);
		spin_DockPadding[SourceFace]->SetMaxValue(100000.0f, FALSE);
		spin_DockPadding[SourceFace]->SetFixedIncrementAmount(1.0f);
	}
	else
	{
		spin_DockPadding[SourceFace]->SetMinValue(-100.0f, FALSE);
		spin_DockPadding[SourceFace]->SetMaxValue(100.0f, FALSE);
		spin_DockPadding[SourceFace]->SetFixedIncrementAmount(0.1f);
	}

	PreviousPaddingType[SourceFace] = NewPaddingType;
	spin_DockPadding[SourceFace]->SetValue(ConvertedValue);

	// pass to our owner control
	Event.Skip();
}


BEGIN_EVENT_TABLE(WxUIDockingPanel, wxScrolledWindow)
	EVT_SPINCTRL(ID_UI_DOCKINGEDITOR_LEFT_PADDING,				WxUIDockingPanel::OnChangeDockingPadding)
	EVT_SPINCTRL(ID_UI_DOCKINGEDITOR_TOP_PADDING,				WxUIDockingPanel::OnChangeDockingPadding)
	EVT_SPINCTRL(ID_UI_DOCKINGEDITOR_RIGHT_PADDING,				WxUIDockingPanel::OnChangeDockingPadding)
	EVT_SPINCTRL(ID_UI_DOCKINGEDITOR_BOTTOM_PADDING,			WxUIDockingPanel::OnChangeDockingPadding)

	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_LEFT,		WxUIDockingPanel::OnChangeDockingEvalType)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_TOP,		WxUIDockingPanel::OnChangeDockingEvalType)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_RIGHT,	WxUIDockingPanel::OnChangeDockingEvalType)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_BOTTOM,	WxUIDockingPanel::OnChangeDockingEvalType)

	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_LEFT_TARGET,				WxUIDockingPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_TOP_TARGET,				WxUIDockingPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_RIGHT_TARGET,				WxUIDockingPanel::OnChangeDockingTarget)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_BOTTOM_TARGET,				WxUIDockingPanel::OnChangeDockingTarget)

	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_LEFT_FACE,					WxUIDockingPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_TOP_FACE,					WxUIDockingPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_RIGHT_FACE,				WxUIDockingPanel::OnChangeDockingFace)
	EVT_COMBOBOX(ID_UI_DOCKINGEDITOR_BOTTOM_FACE,				WxUIDockingPanel::OnChangeDockingFace)
END_EVENT_TABLE()

/** Wx Panel that lets the user change docking settings for the currently selected widget. */
WxUIDockingPanel::WxUIDockingPanel(WxUIEditorBase* InEditor) : 
wxScrolledWindow(InEditor)
{
	wxBoxSizer* BaseVSizer = new wxBoxSizer(wxVERTICAL);
	{
		EditorPanel = new WxUIDockingSetEditorPanel(this);
		BaseVSizer->Add(EditorPanel, 1, wxGROW|wxALL, 0);
	}
	SetSizer(BaseVSizer);

	// Set a scrollbar for this panel.
	EnableScrolling(false, true);
	SetScrollRate(0,1);

	EditorPanel->RefreshEditorPanel(NULL);
}

/** 
 * Sets which widgets are currently selected and updates the panel accordingly.
 *
 * @param SelectedWidgets	The array of currently selected widgets.
 */
void WxUIDockingPanel::SetSelectedWidgets(TArray<UUIObject*> &SelectedWidgets)
{
	// currently we only support changing docking options for 1 widget a time, so only enable the panel if exactly 1 widget is selected.
	if ( SelectedWidgets.Num() == 1 && EditorPanel != NULL )
	{
		Enable(TRUE);
		EditorPanel->RefreshEditorPanel(SelectedWidgets(0));
	}
	else
	{
		Enable(FALSE);
		EditorPanel->RefreshEditorPanel(NULL);
	}
}

/**
* Called when the user changes the value of a "target widget" combo box.  Refreshes and validates the 
* choices available in the "target face" combo for that source face.
*/
void WxUIDockingPanel::OnChangeDockingTarget( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_TARGET);
	PropagateDockingChanges(SourceFace);
}

/**
* Called when the user changes the value of a "target face" combo box.  Refreshes and validates the
* choices available in the "target face" combo for all OTHER source faces.
*/
void WxUIDockingPanel::OnChangeDockingFace( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_FACE);
	PropagateDockingChanges(SourceFace);
}

/**
 * Called when the user changes the scale type for a dock padding.  Applies the updated scale type to the selected widgets
 */
void WxUIDockingPanel::OnChangeDockingEvalType( wxCommandEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_PADDING_EVALTYPE_LEFT);
	PropagateDockingChanges(SourceFace);

}

/**
* Called when the user changes the padding for a dock face. 
*/
void WxUIDockingPanel::OnChangeDockingPadding( wxSpinEvent& Event )
{
	EUIWidgetFace SourceFace = (EUIWidgetFace)(Event.GetId() - ID_UI_DOCKINGEDITOR_LEFT_PADDING);
	PropagateDockingChanges(SourceFace);
}

/**
 * Called when the user changes the value of any docking link value which affects the widget's screen location and/or appearance.  Propagates
 * the change to the widget.
 */
void WxUIDockingPanel::PropagateDockingChanges( EUIWidgetFace SourceFace )
{
	if ( EditorPanel != NULL && EditorPanel->CurrentWidget != NULL && SourceFace < UIFACE_MAX )
	{
		UUIScreenObject* TargetWidget = EditorPanel->GetSelectedDockTarget(SourceFace);
		EUIWidgetFace TargetFace = EditorPanel->GetSelectedDockFace(SourceFace);
		FLOAT Padding = EditorPanel->GetSelectedDockPadding(SourceFace);
		EUIDockPaddingEvalType PaddingType = EditorPanel->GetSelectedDockPaddingType(SourceFace);

		FScopedTransaction Transaction(*LocalizeUI(TEXT("TransEditDocking")));
		FScopedObjectStateChange DockTargetChangeNotifier(EditorPanel->CurrentWidget);

		if ( !EditorPanel->CurrentWidget->SetDockParameters((BYTE)SourceFace, TargetWidget, TargetFace, Padding, PaddingType, TRUE) )
		{
			DockTargetChangeNotifier.CancelEdit();
		}
	}
}











//EOF




