/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"
#include "LevelUtils.h"

enum
{
	DRAWSCALE_NONE,
	DRAWSCALE,
	DRAWSCALE_X,
	DRAWSCALE_Y,
	DRAWSCALE_Z,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxScaleTextCtrl
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void SendDrawScaleToSelectedActors(WxScaleTextCtrl& DrawScaleTextCtrl, INT DrawScaleIndex);

class WxScaleTextCtrl : public wxTextCtrl
{
public:
	WxScaleTextCtrl()
		:	wxTextCtrl(),
		DrawScaleIndex( DRAWSCALE_NONE ),
		Next( NULL ),
		Prev( NULL )
	{}

	WxScaleTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value, const wxPoint& pos, const wxSize& size, long style)
		:	wxTextCtrl( parent, id, value, pos, size, style ),
		DrawScaleIndex( DRAWSCALE_NONE ),
		Next( NULL ),
		Prev( NULL )
	{}

	void Link(INT InDrawScaleIndex, WxScaleTextCtrl* InNext, WxScaleTextCtrl* InPrev)
	{
		DrawScaleIndex = InDrawScaleIndex;
		Next = InNext;
		Prev = InPrev;
	}

private:
	INT DrawScaleIndex;
	WxScaleTextCtrl* Next;
	WxScaleTextCtrl* Prev;

	void OnChar(wxKeyEvent& In)
	{
		switch( In.GetKeyCode() )
		{
		case WXK_UP:
			SendDrawScaleToSelectedActors( *this, DrawScaleIndex );
			check( Prev );
			Prev->SetFocus();
			Next->SetSelection(-1,-1);
			break;
		case WXK_DOWN:
			SendDrawScaleToSelectedActors( *this, DrawScaleIndex );
			check( Next );
			Next->SetFocus();
			Next->SetSelection(-1,-1);
			break;
		default:
			In.Skip();
			break;
		};
	}

	void OnKillFocus(wxFocusEvent& In)
	{
		// Send text when the field loses focus.
		SendDrawScaleToSelectedActors( *this, DrawScaleIndex );
	}

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(WxScaleTextCtrl, wxTextCtrl)
	EVT_CHAR( WxScaleTextCtrl::OnChar )
	EVT_KILL_FOCUS( WxScaleTextCtrl::OnKillFocus )
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static drawscale functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline UBOOL NearlyEqual(FLOAT A, FLOAT B, FLOAT ErrTolerance = KINDA_SMALL_NUMBER)
{
	return Abs( A - B ) < ErrTolerance;
}

static UBOOL GetDrawScaleFromSelectedActors(FLOAT& DrawScale,
											FVector& DrawScale3D,
											UBOOL MultipleDrawScaleValues[4])
{
	for( INT i = 0 ; i < 4 ; ++i )
	{
		MultipleDrawScaleValues[i] = FALSE;
	}

	UBOOL bFoundActor = FALSE;

	for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		if ( !bFoundActor )
		{
			// This is the first actor we've encountered; copy off values.
			bFoundActor = TRUE;
			DrawScale = Actor->DrawScale;
			DrawScale3D = Actor->DrawScale3D;
		}
		else
		{
			if ( !MultipleDrawScaleValues[0] && !NearlyEqual( Actor->DrawScale, DrawScale ) )
			{
				MultipleDrawScaleValues[0] = TRUE;
			}

			for ( INT i = 0 ; i < 3 ; ++i )
			{
				if ( !MultipleDrawScaleValues[i+1] && !NearlyEqual( Actor->DrawScale3D[i], DrawScale3D[i] ) )
				{
					MultipleDrawScaleValues[i+1] = TRUE;
				}
			}

			// Once we've found that all values differ, we can halt.
			if ( MultipleDrawScaleValues[0] && MultipleDrawScaleValues[1] && MultipleDrawScaleValues[2] && MultipleDrawScaleValues[3] )
			{
				break;
			}
		}
	}

	return bFoundActor;
}

static void SendDrawScaleToSelectedActors(WxScaleTextCtrl& DrawScaleTextCtrl, INT DrawScaleIndex)
{
	double DoubleDrawScale;
	const UBOOL bIsNumber = DrawScaleTextCtrl.GetValue().ToDouble( &DoubleDrawScale );

	if( bIsNumber )
	{
		const FLOAT DrawScale = static_cast<FLOAT>( DoubleDrawScale );

		// Fires CALLBACK_LevelDirtied when falling out of scope.
		FScopedLevelDirtied		LevelDirtyCallback;

		for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* Actor = static_cast<AActor*>( *It );
			checkSlow( Actor->IsA(AActor::StaticClass()) );

			if ( Actor != GWorld->GetBrush() )
			{
				UBOOL bUpdateActor = FALSE;
				switch( DrawScaleIndex )
				{
					case DRAWSCALE:
						if ( !NearlyEqual(Actor->DrawScale, DrawScale) )
						{
							Actor->DrawScale = DrawScale;
							bUpdateActor = TRUE;
						}
						break;
					case DRAWSCALE_X:
						if ( !NearlyEqual(Actor->DrawScale3D.X, DrawScale) )
						{
							Actor->DrawScale3D.X = DrawScale;
							bUpdateActor = TRUE;
						}
						break;
					case DRAWSCALE_Y:
						if ( !NearlyEqual(Actor->DrawScale3D.Y, DrawScale) )
						{
							Actor->DrawScale3D.Y = DrawScale;
							bUpdateActor = TRUE;
						}
						break;
					case DRAWSCALE_Z:
						if ( !NearlyEqual(Actor->DrawScale3D.Z, DrawScale) )
						{
							Actor->DrawScale3D.Z = DrawScale;
							bUpdateActor = TRUE;
						}
						break;
					default:
						check( 0 );
						break;
				}

				if ( bUpdateActor )
				{
					Actor->MarkPackageDirty();
					Actor->InvalidateLightingCache();
					Actor->ForceUpdateComponents();
					Actor->PostEditMove( TRUE );

					LevelDirtyCallback.Request();
				}
			}
		}

		if ( LevelDirtyCallback.HasRequests() )
		{
			GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
		}

		DrawScaleTextCtrl.SetSelection(-1,-1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	FIELD_ExecCombo,
	FIELD_Status,
	FIELD_ActorName,
	FIELD_MouseWorldspacePosition,
	FIELD_DrawScale,
	FIELD_DrawScaleX,
	FIELD_DrawScaleY,
	FIELD_DrawScaleZ,
	FIELD_Drag_Grid,
	FIELD_Rotation_Grid,
	FIELD_Scale_Grid,
	FIELD_Autosave,
	FIELD_Buffer,
};

BEGIN_EVENT_TABLE(WxStatusBarStandard, WxStatusBar)
	EVT_SIZE(OnSize)
	EVT_COMMAND( IDCB_DRAG_GRID_TOGGLE, wxEVT_COMMAND_CHECKBOX_CLICKED, WxStatusBarStandard::OnDragGridToggleClick )
	EVT_COMMAND( IDCB_ROTATION_GRID_TOGGLE, wxEVT_COMMAND_CHECKBOX_CLICKED, WxStatusBarStandard::OnRotationGridToggleClick )
	EVT_COMMAND( IDCB_SCALE_GRID_TOGGLE, wxEVT_COMMAND_CHECKBOX_CLICKED, WxStatusBarStandard::OnScaleGridToggleClick )
	EVT_COMMAND( IDCB_AUTOSAVE_TOGGLE, wxEVT_COMMAND_CHECKBOX_CLICKED, WxStatusBarStandard::OnAutosaveToggleClick )
	EVT_TEXT_ENTER( ID_EXEC_COMMAND, WxStatusBarStandard::OnExecComboEnter )
	EVT_TEXT_ENTER( IDTB_DRAWSCALE, WxStatusBarStandard::OnDrawScale )
	EVT_TEXT_ENTER( IDTB_DRAWSCALEX, WxStatusBarStandard::OnDrawScaleX )
	EVT_TEXT_ENTER( IDTB_DRAWSCALEY, WxStatusBarStandard::OnDrawScaleY )
	EVT_TEXT_ENTER( IDTB_DRAWSCALEZ, WxStatusBarStandard::OnDrawScaleZ )
	EVT_COMBOBOX( ID_EXEC_COMMAND, WxStatusBarStandard::OnExecComboSelChange )

	EVT_UPDATE_UI( IDSB_AUTOSAVE, WxStatusBarStandard::OnUpdateUI )
END_EVENT_TABLE()

WxStatusBarStandard::WxStatusBarStandard()
	: WxStatusBar()
{
	ExecCombo = NULL;
	DragGridCB = RotationGridCB = NULL;
	DragGridSB = RotationGridSB = NULL;
	DragGridMB = RotationGridMB = NULL;
	DragGridST = RotationGridST = NULL;

	ScaleGridCB = NULL;
	ScaleGridMB = NULL;
	ScaleGridST = NULL;

	DrawScaleTextCtrl = NULL;
	DrawScaleXTextCtrl = NULL;
	DrawScaleYTextCtrl = NULL;
	DrawScaleZTextCtrl = NULL;

	ActorNameST = NULL;

	AutoSaveMB = NULL;
	AutoSaveCB = NULL;
}

void WxStatusBarStandard::SetUp()
{
	//////////////////////
	// Bitmaps

	wxImage TempDragGrid;
	TempDragGrid.LoadFile( *FString::Printf( TEXT("%swxres\\DragGrid.bmp"), appBaseDir() ), wxBITMAP_TYPE_BMP );
	DragGridB = wxBitmap(TempDragGrid);
	DragGridB.SetMask( new wxMask( DragGridB, wxColor(192,192,192) ) );

	wxImage TempRotationGrid;
	TempRotationGrid.LoadFile( *FString::Printf( TEXT("%swxres\\RotationGrid.bmp"), appBaseDir() ), wxBITMAP_TYPE_BMP );
	RotationGridB = wxBitmap(TempRotationGrid);
	RotationGridB.SetMask( new wxMask( RotationGridB, wxColor(192,192,192) ) );

	//////////////////////
	// Controls

	wxString dummy[1];
	ExecCombo = new WxComboBox( this, ID_EXEC_COMMAND, TEXT(""), wxDefaultPosition, wxDefaultSize, 0, dummy, wxCB_DROPDOWN );
	
	DragGridST = new wxStaticText( this, IDST_DRAG_GRID, TEXT("XXXX"), wxDefaultPosition, wxSize(-1,-1), wxST_NO_AUTORESIZE );
	RotationGridST = new wxStaticText( this, IDST_ROTATION_GRID, TEXT("XXXX"), wxDefaultPosition, wxSize(-1,-1), wxST_NO_AUTORESIZE );
	ScaleGridST = new wxStaticText( this, wxID_ANY, TEXT("XXXXX"), wxDefaultPosition, wxSize(-1,-1), wxST_NO_AUTORESIZE );
	
	DragGridCB = new wxCheckBox( this, IDCB_DRAG_GRID_TOGGLE, TEXT("") );
	RotationGridCB = new wxCheckBox( this, IDCB_ROTATION_GRID_TOGGLE, TEXT("") );
	ScaleGridCB = new wxCheckBox( this, IDCB_SCALE_GRID_TOGGLE, TEXT("") );
	AutoSaveCB = new wxCheckBox( this, IDCB_AUTOSAVE_TOGGLE, TEXT("") );
	
	DragGridSB = new wxStaticBitmap( this, IDSB_DRAG_GRID, DragGridB );
	RotationGridSB = new wxStaticBitmap( this, IDSB_ROTATION_GRID, RotationGridB );
	
	DragGridMB = new WxMenuButton( this, IDPB_DRAG_GRID, &GApp->EditorFrame->DownArrowB, (wxMenu*) GApp->EditorFrame->GetDragGridMenu() );
	RotationGridMB = new WxMenuButton( this, IDPB_ROTATION_GRID, &GApp->EditorFrame->DownArrowB, (wxMenu*) GApp->EditorFrame->GetRotationGridMenu() );
	AutoSaveMB = new WxMenuButton( this, IDPB_AUTOSAVE_INTERVAL, &GApp->EditorFrame->DownArrowB, (wxMenu*) GApp->EditorFrame->GetAutoSaveIntervalMenu() );

	ScaleGridMB = new WxMenuButton( this, IDPB_SCALE_GRID, &GApp->EditorFrame->DownArrowB, (wxMenu*) GApp->EditorFrame->GetScaleGridMenu() );
	
	ActorNameST = new wxStaticText( this, wxID_ANY, TEXT(""));

	DrawScaleTextCtrl = new WxScaleTextCtrl( this, IDTB_DRAWSCALE, TEXT("DrawScaleTextCtrl"), wxDefaultPosition, wxSize( -1, -1 ), wxST_NO_AUTORESIZE );//wxTE_PROCESS_ENTER
	DrawScaleXTextCtrl = new WxScaleTextCtrl( this, IDTB_DRAWSCALEX, TEXT("DrawScaleXTextCtrl"), wxDefaultPosition, wxSize( -1, -1 ), wxST_NO_AUTORESIZE );//wxTE_PROCESS_ENTER
	DrawScaleYTextCtrl = new WxScaleTextCtrl( this, IDTB_DRAWSCALEY, TEXT("DrawScaleYTextCtrl"), wxDefaultPosition, wxSize( -1, -1 ), wxST_NO_AUTORESIZE );//wxTE_PROCESS_ENTER
	DrawScaleZTextCtrl = new WxScaleTextCtrl( this, IDTB_DRAWSCALEZ, TEXT("DrawScaleZTextCtrl"), wxDefaultPosition, wxSize( -1, -1 ), wxST_NO_AUTORESIZE );//wxTE_PROCESS_ENTER
	
	DrawScaleTextCtrl->Link( DRAWSCALE, DrawScaleXTextCtrl, DrawScaleZTextCtrl );
	DrawScaleXTextCtrl->Link( DRAWSCALE_X, DrawScaleYTextCtrl, DrawScaleTextCtrl );
	DrawScaleYTextCtrl->Link( DRAWSCALE_Y, DrawScaleZTextCtrl, DrawScaleXTextCtrl );
	DrawScaleZTextCtrl->Link( DRAWSCALE_Z, DrawScaleTextCtrl, DrawScaleYTextCtrl );

	

	DragGridCB->SetToolTip( *LocalizeUnrealEd("ToolTip_16") );
	RotationGridCB->SetToolTip( *LocalizeUnrealEd("ToolTip_17") );
	ScaleGridCB->SetToolTip( *LocalizeUnrealEd("ToolTip_SnapScaling") );
	AutoSaveCB->SetToolTip( *LocalizeUnrealEd("ToolTip_Autosave") );
	AutoSaveMB->SetToolTip( *LocalizeUnrealEd("ToolTip_AutosaveMenu") );
	DragGridMB->SetToolTip( *LocalizeUnrealEd("ToolTip_18") );
	RotationGridMB->SetToolTip( *LocalizeUnrealEd("ToolTip_19") );
	DrawScaleTextCtrl->SetToolTip( *LocalizeUnrealEd("ToolTip_DrawScale") );
	DrawScaleXTextCtrl->SetToolTip( *LocalizeUnrealEd("ToolTip_DrawScale3DX") );
	DrawScaleYTextCtrl->SetToolTip( *LocalizeUnrealEd("ToolTip_DrawScale3DY") );
	DrawScaleZTextCtrl->SetToolTip( *LocalizeUnrealEd("ToolTip_DrawScale3DZ") );

	//////////////////////
	// Now that we have the controls created, figure out how large each pane should be.

	DragTextWidth = DragGridST->GetRect().GetWidth();
	RotationTextWidth = RotationGridST->GetRect().GetWidth();
	ScaleTextWidth = ScaleGridST->GetRect().GetWidth();

	// Create autosave bitmaps
	AutosaveEnabled = WxBitmap(TEXT("Autosave_Enabled"));
	AutosaveDisabled = WxBitmap(TEXT("Autosave_Disabled"));
	AutosaveSoon = WxBitmap(TEXT("Autosave_Soon"));

	AutoSaveSB = new wxStaticBitmap(this, IDSB_AUTOSAVE, AutosaveEnabled);
	AutoSaveSB->SetSize(wxSize(16,16));


	const INT ScaleWidths = 60;
	const INT ExecComboPane = 350;
	const INT ActorNamePane = 200;
	const INT MousePosPane = 180;
	const INT DragGridPane = 2+ DragGridB.GetWidth() +2+ DragTextWidth +2+ DragGridCB->GetRect().GetWidth() +2+ DragGridMB->GetRect().GetWidth() +1;
	const INT RotationGridPane = 2+ RotationGridB.GetWidth() +2+ RotationTextWidth +2+ RotationGridCB->GetRect().GetWidth() +2+ RotationGridMB->GetRect().GetWidth() +3;
	const INT ScaleGridPane = 2+ ScaleTextWidth +2+ ScaleGridCB->GetRect().GetWidth() +2+ ScaleGridMB->GetRect().GetWidth() +3;
	const INT AutosavePane = 62;
	const INT BufferPane = 20;
	const INT Widths[] = { ExecComboPane, -1, ActorNamePane, MousePosPane, ScaleWidths, ScaleWidths, ScaleWidths, ScaleWidths, DragGridPane, RotationGridPane, ScaleGridPane, AutosavePane, BufferPane };
	SetFieldsCount( sizeof(Widths)/sizeof(INT), Widths );

	//////////////////////
	// Update with initial values and resize everything.

	UpdateUI();

	RefreshPositionAndSize();
}

void WxStatusBarStandard::UpdateUI()
{
	if( !DragGridCB )
	{
		return;
	}

	DragGridCB->SetValue( GEditor->Constraints.GridEnabled );
	RotationGridCB->SetValue( GEditor->Constraints.RotGridEnabled );
	ScaleGridCB->SetValue( GEditor->Constraints.SnapScaleEnabled );

	DragGridST->SetLabel( *FString::Printf( TEXT("%g"), GEditor->Constraints.GetGridSize() ) );

	// For integral rotation snap amounts, we'll just display the number.  For snap amounts with fractions,
	// we'll display a '~' character next to the rounded integer.
	const FLOAT RotationSnapDegrees = GEditor->Constraints.RotGridSize.Pitch / (16384.0f / 90.0f );
	const UBOOL bIsExactValue = ( appFractional( RotationSnapDegrees ) < 0.001f );
	const INT RoundedRotationSnapDegrees = appRound( RotationSnapDegrees );
	RotationGridST->SetLabel(
		*FString::Printf( TEXT("%s%i"),
						  bIsExactValue ? TEXT( "" ) : TEXT( "~" ),
						  RoundedRotationSnapDegrees ) );

	ScaleGridST->SetLabel( *FString::Printf( TEXT("%i%c"), GEditor->Constraints.ScaleGridSize, '%' ));

	FLOAT DrawScale = 1.0f;
	FVector DrawScale3D( 1.0f, 1.0f, 1.0f );
	UBOOL bMultipleDrawScaleValues[4];
	if ( GetDrawScaleFromSelectedActors( DrawScale, DrawScale3D, bMultipleDrawScaleValues ) )
	{
		DrawScaleTextCtrl->SetLabel( bMultipleDrawScaleValues[0] ? *LocalizeUnrealEd("Multiple") : *FString::Printf( TEXT("%.4f"), DrawScale ) );
		DrawScaleXTextCtrl->SetLabel( bMultipleDrawScaleValues[1] ? *LocalizeUnrealEd("Multiple") : *FString::Printf( TEXT("%.4f"), DrawScale3D.X ) );
		DrawScaleYTextCtrl->SetLabel( bMultipleDrawScaleValues[2] ? *LocalizeUnrealEd("Multiple") : *FString::Printf( TEXT("%.4f"), DrawScale3D.Y ) );
		DrawScaleZTextCtrl->SetLabel( bMultipleDrawScaleValues[3] ? *LocalizeUnrealEd("Multiple") : *FString::Printf( TEXT("%.4f"), DrawScale3D.Z ) );
	}
	else
	{
		const FString LocalizedNone( LocalizeUnrealEd("None") );
		DrawScaleTextCtrl->SetLabel( *LocalizedNone );
		DrawScaleXTextCtrl->SetLabel( *LocalizedNone );
		DrawScaleYTextCtrl->SetLabel( *LocalizedNone );
		DrawScaleZTextCtrl->SetLabel( *LocalizedNone );
	}

	// Update Actor Name Static Text.
	USelection* SelectedActors = GEditor->GetSelectedActors();
	const INT NumActors = SelectedActors->Num();
	
	if(NumActors == 0)
	{
		ActorNameST->SetLabel(TEXT(""));
	}
	else if(NumActors == 1)
	{
		AActor* Actor					= SelectedActors->GetTop<AActor>();
		ULevel* Level					= Actor->GetLevel();
		ULevelStreaming* StreamingLevel	= FLevelUtils::FindStreamingLevel( Level ); 

		const FString LevelName( StreamingLevel ? StreamingLevel->PackageName.ToString() : LocalizeUnrealEd( "PersistentLevel" ) );
		const FString LabelString( FString::Printf(TEXT("%s.%s"), *LevelName, *Actor->GetName()) );

		ActorNameST->SetLabel( *LabelString );
		ActorNameST->SetToolTip( *LabelString );
	}
	else
	{
		wxString ActorNameString;
		wxString ActorToolTip;
		
		
		// Loop through all actors and see if they are the same type, if they are then display a more 
		// specific string using their class name.
		UBOOL bActorsHaveSameClass = TRUE;
		FString ActorClassString;

		for ( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
		{
			AActor* NthActor = static_cast<AActor*>( *It );
			checkSlow( NthActor->IsA(AActor::StaticClass()) );

			if (ActorClassString == TEXT(""))
			{
				ActorClassString = NthActor->GetClass()->GetName();
				ActorToolTip = *NthActor->GetName();
			}
			else
			{	
				const INT StrCompare = appStricmp(*ActorClassString, *NthActor->GetClass()->GetName());

				if(StrCompare != 0)
				{
					bActorsHaveSameClass = FALSE;
					break;
				}

				ActorToolTip += TEXT(", ");
				ActorToolTip += *NthActor->GetName();
			}

			
		}
	

		if(bActorsHaveSameClass && ActorClassString.Len() )
		{
			ActorNameString.Printf(LocalizeSecure(LocalizeUnrealEd("ActorsClassSelected"), NumActors, *ActorClassString));
		}
		else
		{
			ActorNameString.Printf(LocalizeSecure(LocalizeUnrealEd("ActorsSelected"), NumActors));
		}

		ActorNameST->SetToolTip(ActorToolTip);
		ActorNameST->SetLabel(ActorNameString);
	}
}


/** Update the position and size of the status bar */
void WxStatusBarStandard::RefreshPositionAndSize()
{
	wxSizeEvent DummyEvent;
	OnSize( DummyEvent );
}


/**
* Overload for the default SetStatusText function.  This overload maps
* any text set to field 0, to field 1 because the command combo box takes up
* field 0.
*/
void WxStatusBarStandard::SetStatusText(const wxString& InText, INT InFieldIdx )
{
	if(InFieldIdx == 0)
	{
		InFieldIdx = 1;
	}

	WxStatusBar::SetStatusText(InText, InFieldIdx);
}

void WxStatusBarStandard::OnDragGridToggleClick( wxCommandEvent& InEvent )
{
	GEditor->Constraints.GridEnabled = !GEditor->Constraints.GridEnabled;
}

void WxStatusBarStandard::OnRotationGridToggleClick( wxCommandEvent& InEvent )
{
	GEditor->Constraints.RotGridEnabled = !GEditor->Constraints.RotGridEnabled;
}

void WxStatusBarStandard::OnScaleGridToggleClick( wxCommandEvent& InEvent )
{
	GEditor->Constraints.SnapScaleEnabled = !GEditor->Constraints.SnapScaleEnabled;
}

void WxStatusBarStandard::OnAutosaveToggleClick( wxCommandEvent& InEvent )
{
	GUnrealEd->AutoSave = !GUnrealEd->AutoSave;

	//reset the counter if autosave has been enabled
	if (GUnrealEd->AutoSave) 
	{
		GUnrealEd->AutosaveCount = 0;
	}
}

void WxStatusBarStandard::OnSize( wxSizeEvent& InEvent )
{
	wxRect rect;

	//////////////////////
	// Exec combo

	GetFieldRect( FIELD_ExecCombo, rect );
	ExecCombo->SetSize( rect.x+2, rect.y+2, rect.GetWidth()-4, rect.GetHeight()-4 );

	//////////////////////
	// DrawScale text controls.

	if( DrawScaleTextCtrl )
	{
		GetFieldRect( FIELD_DrawScale, rect );
		DrawScaleTextCtrl->SetSize( rect.x, rect.y, rect.GetWidth(), rect.GetHeight() );
	}
	if( DrawScaleXTextCtrl )
	{
		GetFieldRect( FIELD_DrawScaleX, rect );
		DrawScaleXTextCtrl->SetSize( rect.x, rect.y, rect.GetWidth(), rect.GetHeight() );
	}
	if( DrawScaleYTextCtrl )
	{
		GetFieldRect( FIELD_DrawScaleY, rect );
		DrawScaleYTextCtrl->SetSize( rect.x, rect.y, rect.GetWidth(), rect.GetHeight() );
	}
	if( DrawScaleZTextCtrl )
	{
		GetFieldRect( FIELD_DrawScaleZ, rect );
		DrawScaleZTextCtrl->SetSize( rect.x, rect.y, rect.GetWidth(), rect.GetHeight() );
	}

	//////////////////////
	// Drag grid

	if( DragGridSB )
	{
		GetFieldRect( FIELD_Drag_Grid, rect );

		INT Left = rect.x + 2;
		DragGridSB->SetSize( Left, rect.y+2, DragGridB.GetWidth(), rect.height-4 );
		Left += DragGridB.GetWidth() + 2;
		DragGridST->SetSize( Left, rect.y+2, DragTextWidth, rect.height-4 );
		Left += DragTextWidth + 2;
		DragGridCB->SetSize( Left, rect.y+2, -1, rect.height-4 );
		Left += DragGridCB->GetSize().GetWidth() + 2;
		DragGridMB->SetSize( Left, rect.y+1, 13, rect.height-2 );
	}

	//////////////////////
	// Rotation grid

	if( RotationGridSB )
	{
		GetFieldRect( FIELD_Rotation_Grid, rect );

		INT Left = rect.x + 2;
		RotationGridSB->SetSize( Left, rect.y+2, RotationGridB.GetWidth(), rect.height-4 );
		Left += RotationGridB.GetWidth() + 2;
		RotationGridST->SetSize( Left, rect.y+2, RotationTextWidth, rect.height-4 );
		Left += RotationTextWidth + 2;
		RotationGridCB->SetSize( Left, rect.y+2, -1, rect.height-4 );
		Left += RotationGridCB->GetSize().GetWidth() + 2;
		RotationGridMB->SetSize( Left, rect.y+1, 13, rect.height-2 );
	}

	//////////////////////
	// Scale grid
	if(ScaleGridST)
	{
		GetFieldRect( FIELD_Scale_Grid, rect );

		INT Left = rect.x + 2;
		ScaleGridST->SetSize( Left, rect.y+2, ScaleTextWidth, rect.height-4 );
		Left += ScaleTextWidth + 2;
		ScaleGridCB->SetSize( Left, rect.y+2, -1, rect.height-4 );
		Left += ScaleGridCB->GetSize().GetWidth() + 2;
		ScaleGridMB->SetSize( Left, rect.y+1, 13, rect.height-2 );
	}

	//////////////////////
	// Selected Actors
	if(ActorNameST)
	{
		GetFieldRect( FIELD_ActorName, rect );

		wxSize LocalSize = ActorNameST->GetSize();
		ActorNameST->SetSize(rect.x, rect.GetY() + (rect.height - LocalSize.GetHeight()) / 2, rect.width, LocalSize.GetHeight());
	}

	INT Left = 0;

	// Autosave Static Bitmap
	if(AutoSaveSB)
	{
		GetFieldRect( FIELD_Autosave, rect );
		Left = rect.x + 2;
		AutoSaveSB->SetSize(Left, 1+rect.GetY() + (rect.height - 16) / 2, 16, rect.height-4);
		Left += AutoSaveSB->GetSize().GetWidth() + 12;
	}

	// Autosave Check Box
	if(AutoSaveCB)
	{
		AutoSaveCB->SetSize(Left, 1+rect.GetY() + (rect.height - 16) / 2, 16, rect.height-4);
		Left += AutoSaveCB->GetSize().GetWidth();
	}

	// Autosave Menu Bar
	if (AutoSaveMB) 
	{
		AutoSaveMB->SetSize(Left, 1+rect.GetY() + (rect.height - 16) / 2, 13, rect.height-2);
	}
}

void WxStatusBarStandard::OnDrawScale( wxCommandEvent& In )
{
	SendDrawScaleToSelectedActors( *DrawScaleTextCtrl, DRAWSCALE );
}

void WxStatusBarStandard::OnDrawScaleX( wxCommandEvent& In )
{
	SendDrawScaleToSelectedActors( *DrawScaleXTextCtrl, DRAWSCALE_X );
}

void WxStatusBarStandard::OnDrawScaleY( wxCommandEvent& In )
{
	SendDrawScaleToSelectedActors( *DrawScaleYTextCtrl, DRAWSCALE_Y );
}

void WxStatusBarStandard::OnDrawScaleZ( wxCommandEvent& In )
{
	SendDrawScaleToSelectedActors( *DrawScaleZTextCtrl, DRAWSCALE_Z );
}

void WxStatusBarStandard::OnExecComboEnter( wxCommandEvent& In )
{
	const FString exec = (const TCHAR*)ExecCombo->GetValue();
	GEngine->Exec( *exec );

	AddToExecCombo( exec );
}

void WxStatusBarStandard::OnExecComboSelChange( wxCommandEvent& In )
{
	const FString exec = (const TCHAR*)ExecCombo->GetString( ExecCombo->GetSelection() );
	GEngine->Exec( *exec );
}

void WxStatusBarStandard::AddToExecCombo( const FString& InExec )
{
	// If the string isn't already in the combo box list, add it.
	if( ExecCombo->FindString( *InExec ) == -1 )
	{
		ExecCombo->Append( *InExec );
	}
}

/**
 * Sets the mouse worldspace position text field to the text passed in.
 *
 * @param StatusText	 String to use as the new text for the worldspace position field.
 */
void WxStatusBarStandard::SetMouseWorldspacePositionText( const TCHAR* StatusText )
{
	SetStatusText( StatusText, FIELD_MouseWorldspacePosition );
}

void WxStatusBarStandard::OnUpdateUI(wxUpdateUIEvent &Event)
{
	// Update autosave image
	static INT AutosaveSeconds = -1;
	const UBOOL bCanAutosave = GUnrealEd->CanAutosave();

	AutoSaveCB->SetValue(GUnrealEd->AutoSave);

	if(bCanAutosave)
	{
		const UBOOL bAutosaveSoon = GUnrealEd->AutoSaveSoon();

		if(bAutosaveSoon)
		{
			if(AutoSaveSB->GetBitmap().GetHandle() != AutosaveSoon.GetHandle())
			{
				AutoSaveSB->SetBitmap(AutosaveSoon);
			}

			INT NewSeconds = GUnrealEd->GetTimeTillAutosave();
			if(AutosaveSeconds != NewSeconds)
			{
				AutosaveSeconds = NewSeconds;
				INT Sec = AutosaveSeconds % 60;

				AutoSaveSB->SetToolTip(*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Autosave_Soon"), Sec)));
			}
		}
		else
		{
			if(AutoSaveSB->GetBitmap().GetHandle() != AutosaveEnabled.GetHandle())
			{
				AutoSaveSB->SetBitmap(AutosaveEnabled);
			}

			INT NewSeconds = GUnrealEd->GetTimeTillAutosave();
			if(AutosaveSeconds != NewSeconds)
			{
				AutosaveSeconds = NewSeconds;

				INT Min = AutosaveSeconds / 60;
				INT Sec = AutosaveSeconds % 60;

				AutoSaveSB->SetToolTip(*FString::Printf(LocalizeSecure(LocalizeUnrealEd("Autosave_Enabled"), Min,Sec)));
			}
		}
	}
	else
	{
		if(AutoSaveSB->GetBitmap().GetHandle() != AutosaveDisabled.GetHandle())
		{
			AutoSaveSB->SetBitmap(AutosaveDisabled);
			AutoSaveSB->SetToolTip(*LocalizeUnrealEd("Autosave_Disabled"));
		}
	}
}
