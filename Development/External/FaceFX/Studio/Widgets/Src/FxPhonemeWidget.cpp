//------------------------------------------------------------------------------
// A widget for editing phoneme and word timings.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxStudioApp.h"
#include "FxMath.h"
#include "FxPhonemeWidget.h"
#include "FxPhonWordList.h"
#include "FxAnimUserData.h"
#include "FxColourMapping.h"
#include "FxStudioOptions.h"
#include "FxVM.h"
#include "FxStudioOptions.h"
#include "FxStudioAnimPlayer.h"
#include "FxConsoleVariable.h"
#include "FxTearoffWindow.h"

#include "FxReanalyzeSelectionDialog.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/image.h"
	#include "wx/valgen.h"
	#include "wx/colordlg.h"
	#include "wx/confbase.h"
	#include "wx/fileconf.h"
#endif

namespace OC3Ent
{

namespace Face
{

#define PHONMENU_MODIFIER 9285

WX_IMPLEMENT_DYNAMIC_CLASS( FxPhonemeWidget, wxWindow )

BEGIN_EVENT_TABLE( FxPhonemeWidget, wxWindow )
	EVT_MENU( MenuID_PhonemeWidgetResetView, FxPhonemeWidget::OnResetView )
	EVT_MENU( MenuID_PhonemeWidgetPlaySelection, FxPhonemeWidget::OnPlaySelection )
	EVT_MENU( MenuID_PhonemeWidgetDeleteSelection, FxPhonemeWidget::OnDeleteSelection )
	EVT_MENU( MenuID_PhonemeWidgetGroupSelection, FxPhonemeWidget::OnGroupSelection )
	EVT_MENU( MenuID_PhonemeWidgetUngroupSelection, FxPhonemeWidget::OnUngroupSelection )
	EVT_MENU( MenuID_PhonemeWidgetInsertPhoneme, FxPhonemeWidget::OnInsertPhoneme )
	EVT_MENU( MenuID_PhonemeWidgetAppendPhoneme, FxPhonemeWidget::OnAppendPhoneme )
	EVT_MENU( MenuID_PhonemeWidgetOptions, FxPhonemeWidget::OnOptions )
	EVT_MENU( MenuID_PhonemeWidgetCut, FxPhonemeWidget::OnCut )
	EVT_MENU( MenuID_PhonemeWidgetCopy, FxPhonemeWidget::OnCopy )
	EVT_MENU( MenuID_PhonemeWidgetPaste, FxPhonemeWidget::OnPaste )
	EVT_MENU( MenuID_PhonemeWidgetUndo, FxPhonemeWidget::OnUndo )
	EVT_MENU( MenuID_PhonemeWidgetRedo, FxPhonemeWidget::OnRedo )
	EVT_MENU( MenuID_PhonemeWidgetZoomSelection, FxPhonemeWidget::OnZoomSelection )
	EVT_MENU( MenuID_PhonemeWidgetReanalyzeSelection, FxPhonemeWidget::OnReanalyzeSelection )
	EVT_MENU( MenuID_PhonemeWidgetQuickChangePhoneme, FxPhonemeWidget::OnQuickChangePhoneme )

	EVT_SET_FOCUS( FxPhonemeWidget::OnGetFocus )
	EVT_KILL_FOCUS( FxPhonemeWidget::OnLoseFocus )

	EVT_HELP_RANGE( wxID_ANY, wxID_HIGHEST, FxPhonemeWidget::OnHelp )

	EVT_PAINT( FxPhonemeWidget::OnPaint )
	EVT_MOUSEWHEEL( FxPhonemeWidget::OnMouseWheel )
	EVT_MOTION( FxPhonemeWidget::OnMouseMove )
	EVT_LEFT_DOWN( FxPhonemeWidget::OnLButtonDown )
	EVT_LEFT_UP( FxPhonemeWidget::OnLButtonUp )
	EVT_LEFT_DCLICK( FxPhonemeWidget::OnLeftDoubleClick )
	EVT_RIGHT_UP( FxPhonemeWidget::OnRButtonClick )
	EVT_MIDDLE_DOWN( FxPhonemeWidget::OnMiddleButtonDown )

	EVT_MENU_RANGE( PHON_FIRST + PHONMENU_MODIFIER, PHON_LAST + PHONMENU_MODIFIER, FxPhonemeWidget::OnPhonemeMenu )
END_EVENT_TABLE()

FxArray<FxPhonInfo> FxPhonemeWidget::_copyPhoneBuffer;
FxArray<FxWordInfo> FxPhonemeWidget::_copyWordBuffer;

FxPhonemeWidget::FxPhonemeWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: Super( parent, -1, wxDefaultPosition, wxSize(0, 50), wxFULL_REPAINT_ON_RESIZE )
	, FxWidget( mediator )
	, _phonsOnTop( FxTrue )
	, _ungroupedPhonsFillBar( FxFalse )
	, _ySplit( GetClientRect().GetHeight() / 2 )
	, _minTime( 0.0f )
	, _maxTime( 1.0f )
	, _currentTime( FxInvalidValue )
	, _mouseWheelZoomFactor( 1.5f )
	, _zoomFactor( 1.035f )
	, _pAnim( NULL )
	, _pPhonList( NULL )
	, _mouseAction( MOUSE_NONE )
	, _index( FxInvalidIndex )
	, _hasFocus( FxFalse )
	, pl_realtimeupdate(FxVM::FindConsoleVariable("pl_realtimeupdate"))
	, a_audiomin(FxVM::FindConsoleVariable("a_audiomin"))
	, a_audiomax(FxVM::FindConsoleVariable("a_audiomax"))
{
	FxAssert(pl_realtimeupdate);
	FxAssert(a_audiomin);
	FxAssert(a_audiomax);

	FxTimeManager::RegisterSubscriber( this );
	Initialize();

	static const int numEntries = 13;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set( wxACCEL_CTRL, (FxInt32)'R', MenuID_PhonemeWidgetResetView );
	entries[1].Set( wxACCEL_NORMAL, WXK_SPACE, MenuID_PhonemeWidgetPlaySelection );
	entries[2].Set( wxACCEL_NORMAL, WXK_DELETE, MenuID_PhonemeWidgetDeleteSelection );
	entries[3].Set( wxACCEL_NORMAL, WXK_INSERT, MenuID_PhonemeWidgetInsertPhoneme );
	entries[4].Set( wxACCEL_CTRL, (FxInt32)'G', MenuID_PhonemeWidgetGroupSelection );
	entries[5].Set( wxACCEL_CTRL, (FxInt32)'U', MenuID_PhonemeWidgetUngroupSelection );
	entries[6].Set( wxACCEL_CTRL, (FxInt32)'P', MenuID_PhonemeWidgetOptions );
	entries[7].Set( wxACCEL_CTRL, (FxInt32)'W', MenuID_PhonemeWidgetZoomSelection );
	entries[8].Set( wxACCEL_CTRL, (FxInt32)'Q', MenuID_PhonemeWidgetReanalyzeSelection );
	entries[9].Set( wxACCEL_CTRL, (FxInt32)'X', MenuID_PhonemeWidgetCut );
	entries[10].Set( wxACCEL_CTRL, (FxInt32)'C', MenuID_PhonemeWidgetCopy );
	entries[11].Set( wxACCEL_CTRL,(FxInt32)'V', MenuID_PhonemeWidgetPaste );
	entries[12].Set( wxACCEL_CTRL, (FxInt32)'D', MenuID_PhonemeWidgetQuickChangePhoneme);
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator( numEntries, entries );
	SetAcceleratorTable( accelerator );
}

FxPhonemeWidget::~FxPhonemeWidget()
{
}

void FxPhonemeWidget::OnNotifyTimeChange()
{
	FxTimeManager::GetTimes( _minTime, _maxTime );
	Refresh( FALSE );
	Update();
}

FxReal FxPhonemeWidget::GetMinimumTime()
{
	FxReal retval = 0.0f;
	if( _pPhonList && _pPhonList->GetNumPhonemes() > 0 )
	{
		retval = _pPhonList->GetPhonemeStartTime( 0 );
	}
	return retval;
}

FxReal FxPhonemeWidget::GetMaximumTime()
{
	FxReal retval = 0.0f;
	if( _pPhonList && _pPhonList->GetNumPhonemes() > 0 )
	{
		retval = _pPhonList->GetPhonemeEndTime( _pPhonList->GetNumPhonemes() - 1 );
	}
	return retval;
}

FxInt32 FxPhonemeWidget::GetPaintPriority()
{
	return 3;
}

void FxPhonemeWidget::OnAppShutdown( FxWidget* FxUnused(sender) )
{
	FxTimeManager::UnregisterSubscriber(this);
}

void FxPhonemeWidget::
OnAnimChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim )
{
	_pAnim = anim;
	_pPhonList = NULL;
	if( _pAnim )
	{
		FxAnimUserData* pAnimUserData = reinterpret_cast<FxAnimUserData*>(_pAnim->GetUserData());
		if( pAnimUserData )
		{
			_pPhonList = pAnimUserData->GetPhonemeWordListPointer();
		}
	}
	Refresh( FALSE );
}

void FxPhonemeWidget::OnTimeChanged(FxWidget* FxUnused(sender), FxReal newTime )
{
	_currentTime = newTime;
	Refresh( FALSE );
	Update();
}

void FxPhonemeWidget::OnRefresh( FxWidget* FxUnused(sender) )
{
	Refresh(FALSE);
}

void FxPhonemeWidget::OnSerializeOptions( FxWidget* FxUnused(sender), wxFileConfig* config, FxBool isLoading )
{
	if( isLoading )
	{
		FxSize numColours = _phonClassInfo.Length() - 1;
		for( FxSize i = 0; i < numColours; ++i )
		{
			_phonClassInfo[i+1].colour = FxStringToColour(config->Read( wxT("/PhonemeWidget/")+_phonClassInfo[i+1].label, FxColourToString(_phonClassInfo[i+1].defaultColour)));
		}
		config->Read(wxT("/PhonemeWidget/UngroupedPhonsFillBar"), &_ungroupedPhonsFillBar, FxFalse);
	}
	else
	{
		FxSize numColours = _phonClassInfo.Length() - 1;
		for( FxSize i = 0; i < numColours; ++i )
		{
			config->Write( wxT("/PhonemeWidget/")+_phonClassInfo[i+1].label, FxColourToString(_phonClassInfo[i+1].colour) );
		}
		config->Write( wxT("/PhonemeWidget/UngroupedPhonsFillBar"), _ungroupedPhonsFillBar );
	}
}

void FxPhonemeWidget::OnMouseWheel( wxMouseEvent& event )
{
	eZoomDir zoomDir = ZOOM_OUT;
	if( event.GetWheelRotation() < 0 )
	{
		zoomDir = ZOOM_IN;
	}
	Zoom(event.GetPosition(), _mouseWheelZoomFactor, zoomDir);
}

void FxPhonemeWidget::OnMouseMove( wxMouseEvent& event )
{
	wxPoint thisPos = event.GetPosition();
	if( _pPhonList )
	{
		wxRect clientRect = GetClientRect();
		// If the user isn't holding the movement modifier
		//	if( !event.ControlDown() )
		//	{
		// Check if we're over a phoneme boundary to change the cursor
		if( _mouseAction == MOUSE_NONE )
		{
			FxBool foundPhon = HitTestPhonBoundaries( thisPos, &_index, &_side );
			FxBool foundWord = HitTestWordBoundaries( thisPos );
			if( event.ControlDown() )
			{
				SetCursor( wxCursor( foundPhon ? (_side == START ? wxCURSOR_POINT_LEFT : wxCURSOR_POINT_RIGHT) : wxCURSOR_ARROW ) );	
			}
			else
			{
				SetCursor( wxCursor( ( foundPhon || foundWord )? wxCURSOR_SIZEWE : wxCURSOR_ARROW ) );
			}
		}

		// Handle phoneme editing
		if( _mouseAction == MOUSE_EDIT_PHONEMEBOUNDARY && _index != FxInvalidIndex )
		{
			if( event.ControlDown() )
			{
				if( _side == START )
				{
					_pPhonList->PushPhonemeStartTime( _index, 
						FxTimeManager::CoordToTime( thisPos.x, clientRect ) );
				}
				else
				{
					_pPhonList->PushPhonemeEndTime( _index, 
						FxTimeManager::CoordToTime( thisPos.x, clientRect ) );
				}			
			}
			else
			{
				if( _side == START )
				{
					_pPhonList->ModifyPhonemeStartTime( _index, 
						FxTimeManager::CoordToTime( thisPos.x, clientRect ) );
				}
				else
				{
					_pPhonList->ModifyPhonemeEndTime( _index, 
						FxTimeManager::CoordToTime( thisPos.x, clientRect ) );
				}
			}

			Refresh( FALSE );
			if( static_cast<FxBool>(*pl_realtimeupdate) )
			{
				DispatchPhonemesChanged();
				Update();
			}
		}
		// Handle word editing
		else if( _mouseAction == MOUSE_EDIT_WORDBOUNDARY && _index != FxInvalidIndex )
		{
			if( _side == START )
			{
				_pPhonList->ModifyWordStartTime( _index,
					FxTimeManager::CoordToTime( thisPos.x, clientRect ) );			
			}
			else
			{
				_pPhonList->ModifyWordEndTime( _index,
					FxTimeManager::CoordToTime( thisPos.x, clientRect ) );
			}
			Refresh( FALSE );
			if( static_cast<FxBool>(*pl_realtimeupdate) )
			{
				DispatchPhonemesChanged();
				Update();
			}
		}
		//	}
		//	else
		//	{
		//		SetCursor( wxCursor( wxCURSOR_ARROW ) );
		//	}

		// Zoom the view
		if( event.MiddleIsDown() && event.LeftIsDown() )
		{
			eZoomDir zoomDir = ZOOM_NONE;
			if( thisPos.x < _lastPos.x )
			{
				zoomDir = ZOOM_IN;
			}
			else if( thisPos.x > _lastPos.x )
			{
				zoomDir = ZOOM_OUT;
			}
			// Zoom around the center.
			Zoom( wxPoint(clientRect.GetWidth() / 2, clientRect.GetHeight() / 2), 
				_zoomFactor, zoomDir );
		}
		// Pan the view
		else if( event.MiddleIsDown() )
		{
			FxReal deltaTime  = FxTimeManager::CoordToTime( thisPos.x, clientRect ) -
				FxTimeManager::CoordToTime( _lastPos.x, clientRect );

			FxTimeManager::RequestTimeChange( _minTime - deltaTime, _maxTime - deltaTime );
		}
	}
	_lastPos = thisPos;
}

// Left button down
void FxPhonemeWidget::OnLButtonDown( wxMouseEvent& event )
{
	SetFocus();
	if( !HasCapture() )
	{
		CaptureMouse();
	}

	if( _pPhonList )
	{
		if( !event.ShiftDown() )
		{
			if( HitTestPhonBoundaries( event.GetPosition(), &_index, &_side ) )
			{
				_mouseAction = MOUSE_EDIT_PHONEMEBOUNDARY;
				FlagNoUnload();
				FxVM::ExecuteCommand("phonList -p");
			}
			else if( HitTestWordBoundaries( event.GetPosition(), &_index, &_side ) )
			{
				_mouseAction = MOUSE_EDIT_WORDBOUNDARY;
				FlagNoUnload();
				FxVM::ExecuteCommand("phonList -p");
			}
			else if( HitTestPhonemes( event.GetPosition(), &_index ) )
			{
				FxBool state = _pPhonList->GetPhonemeSelected( _index );
				_pPhonList->ClearSelection();
				_pPhonList->StartSelection( FxInvalidIndex, _index, !state );
			}
			else if( HitTestWords( event.GetPosition(), &_index ) )
			{
				FxBool state = _pPhonList->GetWordSelected( _index );
				_pPhonList->ClearSelection();
				_pPhonList->StartSelection( _index, FxInvalidIndex, !state );
			}
		}
		else if( event.ShiftDown() )
		{
			FxBool selection = _pPhonList->HasSelection();
			_pPhonList->ClearSelection();
			if( HitTestPhonemes( event.GetPosition(), &_index ) )
			{
				if( selection )
				{
					_pPhonList->EndSelection( FxInvalidIndex, _index );
				}
				else
				{
					_pPhonList->StartSelection( FxInvalidIndex, _index );
				}
			}
			else if( HitTestWords( event.GetPosition(), &_index ) )
			{
				if( selection )
				{
					_pPhonList->EndSelection( _index, FxInvalidIndex );
				}
				else
				{
					_pPhonList->StartSelection( _index, FxInvalidIndex );
				}
			}
		}
		Refresh( FALSE );
	}
}

// Left button up
void FxPhonemeWidget::OnLButtonUp( wxMouseEvent& FxUnused(event) )
{
	if( HasCapture() )
	{
		ReleaseMouse();
	}
	if( _mouseAction != MOUSE_NONE )
	{
		DispatchPhonemesChanged();
	}
	_mouseAction = MOUSE_NONE;
	_index = FxInvalidIndex;
	Refresh( FALSE );
}

// Left button doubleclick
void FxPhonemeWidget::OnLeftDoubleClick( wxMouseEvent& event )
{
	SetFocus();
	if( ! _pPhonList->HasSelection() )
	{
		OnLButtonDown(event);
	}
	wxCommandEvent temp;
	OnPlaySelection( temp );
}

// Right button down
void FxPhonemeWidget::OnRButtonClick( wxMouseEvent& event )
{
	SetFocus();
	Refresh(FALSE);

	if( _pPhonList )
	{
		wxMenu popupMenu;
		if( HitTestPhonemes( event.GetPosition(), &_index ) )
		{
			if( !_pPhonList->GetPhonemeSelected( _index ) )
			{
				_pPhonList->ClearSelection();
				_pPhonList->SetPhonemeSelected( _index, FxTrue );
				Refresh( FALSE );
			}
			popupMenu.Append( MenuID_PhonemeWidgetCut, _("Cu&t\tCtrl+X"), _("Cut the selection") );
			popupMenu.Enable( MenuID_PhonemeWidgetCut, _pPhonList->HasSelection() );
			popupMenu.Append( MenuID_PhonemeWidgetCopy, _("&Copy\tCtrl+C"), _("Copy the selection") );
			popupMenu.Enable( MenuID_PhonemeWidgetCopy, _pPhonList->HasSelection() );
			popupMenu.Append( MenuID_PhonemeWidgetPaste, _("&Paste\tCtrl+V"), _("Paste the cut/copied data") );
			popupMenu.Enable( MenuID_PhonemeWidgetPaste, _copyPhoneBuffer.Length()!=0 );
			popupMenu.AppendSeparator();
			popupMenu.Append( MenuID_PhonemeWidgetDeleteSelection, _("&Delete selection\tDel"), _("Delete the currently selected phoneme(s)") );
			popupMenu.Append( MenuID_PhonemeWidgetInsertPhoneme, _("&Insert phoneme\tIns"), _("Insert a phoneme") );

			// If we clicked the last phoneme, add the append option.
			if( _index == _pPhonList->GetNumPhonemes() - 1 )
			{
				popupMenu.Append( MenuID_PhonemeWidgetAppendPhoneme, _("&Append phoneme"), _("Append a phoneme to the end of the phoneme list.") );
			}

			wxMenu* vowel = new wxMenu();
			wxMenu* diphthong = new wxMenu();
			wxMenu* fricative = new wxMenu();
			wxMenu* nasal = new wxMenu();
			wxMenu* approximate = new wxMenu();
			wxMenu* plosive = new wxMenu();
			wxMenu* trill = new wxMenu();

			wxFont unicodeFont( 8.0f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode") );

			FxPhoneticAlphabet phonAlphabet = FxStudioOptions::GetPhoneticAlphabet();

			for( FxSize i = PHON_FIRST; i < PHON_LAST; ++i )
			{
				wxString label;
				switch( phonAlphabet )
				{
				case PA_IPA:
					label = FxGetPhonemeInfo(i).ipa;
					break;
				case PA_SAMPA:
					label = FxGetPhonemeInfo(i).sampa;
					break;
				default:
				case PA_DEFAULT:
					label = FxGetPhonemeInfo(i).talkback;
					break;
				};
				label.Append( wxT("\t [") );
				label.Append( FxGetPhonemeInfo(i).sampleWords );
				label.Append( wxT("]") );


				switch( FxGetPhonemeInfo(i).phonClass )
				{
				case PHONCLASS_VOWEL:
					{
						wxMenuItem* menuItem = new wxMenuItem(vowel, i + PHONMENU_MODIFIER, label);
						if( phonAlphabet == PA_IPA )
						{
#ifdef WIN32
							menuItem->SetFont(unicodeFont);
#endif
						}
						vowel->Append( menuItem );
					}
					break;
				case PHONCLASS_DIPHTHONG:
					{
						wxMenuItem* menuItem = new wxMenuItem(vowel, i + PHONMENU_MODIFIER, label);
						if( phonAlphabet == PA_IPA )
						{
#ifdef WIN32
							menuItem->SetFont(unicodeFont);
#endif
						}
						diphthong->Append( menuItem );
					}
					break;
				case PHONCLASS_FRICATIVE:
					{
						wxMenuItem* menuItem = new wxMenuItem(vowel, i + PHONMENU_MODIFIER, label);
						if( phonAlphabet == PA_IPA )
						{
#ifdef WIN32
							menuItem->SetFont(unicodeFont);
#endif
						}
						fricative->Append( menuItem );
					}
					break;
				case PHONCLASS_NASAL:
					{
						wxMenuItem* menuItem = new wxMenuItem(vowel, i + PHONMENU_MODIFIER, label);
						if( phonAlphabet == PA_IPA )
						{
#ifdef WIN32
							menuItem->SetFont(unicodeFont);
#endif
						}
						nasal->Append( menuItem );
					}
					break;
				case PHONCLASS_APPROXIMATE:
					{
						wxMenuItem* menuItem = new wxMenuItem(vowel, i + PHONMENU_MODIFIER, label);
						if( phonAlphabet == PA_IPA )
						{
#ifdef WIN32
							menuItem->SetFont(unicodeFont);
#endif
						}
						approximate->Append( menuItem );
					}
					break;
				case PHONCLASS_PLOSIVE:
					{
						wxMenuItem* menuItem = new wxMenuItem(vowel, i + PHONMENU_MODIFIER, label);
						if( phonAlphabet == PA_IPA )
						{
#ifdef WIN32
							menuItem->SetFont(unicodeFont);
#endif
						}
						plosive->Append( menuItem );
					}
					break;
				case PHONCLASS_TRILL:
					{
						wxMenuItem* menuItem = new wxMenuItem(vowel, i + PHONMENU_MODIFIER, label);
						if( phonAlphabet == PA_IPA )
						{
#ifdef WIN32
							menuItem->SetFont(unicodeFont);
#endif
						}
						trill->Append( menuItem );

					}
				default:
					break;
				};
			}

			wxMenu* changePhoneme = new wxMenu();

			// Magic numbers don't matter here.  Since we can't select the 
			// submenus themselves, we don't need to worry about processing 
			// a menu message with an unknown ID.
			changePhoneme->Append( 4573, _phonClassInfo[PHONCLASS_VOWEL].label, vowel );
			changePhoneme->Append( 4574, _phonClassInfo[PHONCLASS_DIPHTHONG].label, diphthong );
			changePhoneme->Append( 4575, _phonClassInfo[PHONCLASS_FRICATIVE].label, fricative );
			changePhoneme->Append( 4576, _phonClassInfo[PHONCLASS_NASAL].label, nasal );
			changePhoneme->Append( 4577, _phonClassInfo[PHONCLASS_APPROXIMATE].label, approximate );
			changePhoneme->Append( 4578, _phonClassInfo[PHONCLASS_PLOSIVE].label, plosive );
			changePhoneme->Append( 4579, _phonClassInfo[PHONCLASS_TRILL].label, trill);
			changePhoneme->Append( PHON_SIL + PHONMENU_MODIFIER, _("Silence"), _("(silence)") );

			popupMenu.Append( 4579, _("Change phoneme"), changePhoneme );
			popupMenu.Append( MenuID_PhonemeWidgetQuickChangePhoneme, _("Quick change phoneme...\tCtrl+D"), _("Quick change dialog box") );

			popupMenu.Append( MenuID_PhonemeWidgetGroupSelection, _("&Group to word\tCtrl+G"), _("Group the selected phonemes to a word") );

			popupMenu.AppendSeparator();
			popupMenu.Append( MenuID_PhonemeWidgetPlaySelection, _("P&lay phoneme\tSpace"), _("Play the audio for this phoneme") );
		}
		else if( HitTestWords( event.GetPosition(), &_index ) )
		{
			// Do stuff for words
			if( !_pPhonList->GetWordSelected( _index ) )
			{
				_pPhonList->ClearSelection();
				_pPhonList->SetWordSelected( _index, FxTrue );
				Refresh( FALSE );
			}

			popupMenu.Append( MenuID_PhonemeWidgetCut, _("Cu&t\tCtrl+X"), _("Cut the selection") );
			popupMenu.Enable( MenuID_PhonemeWidgetCut, _pPhonList->HasSelection() );
			popupMenu.Append( MenuID_PhonemeWidgetCopy, _("&Copy\tCtrl+C"), _("Copy the selection") );
			popupMenu.Enable( MenuID_PhonemeWidgetCopy, _pPhonList->HasSelection() );
			popupMenu.Append( MenuID_PhonemeWidgetPaste, _("&Paste\tCtrl+V"), _("Paste the cut/copied data") );
			popupMenu.Enable( MenuID_PhonemeWidgetPaste, _copyPhoneBuffer.Length()!=0 );
			popupMenu.AppendSeparator();
			popupMenu.Append( MenuID_PhonemeWidgetDeleteSelection, _("&Delete selection\tDel" ), _("Remove the selected word and associated phonemes") );
			popupMenu.Append( MenuID_PhonemeWidgetInsertPhoneme, _("&Insert phoneme\tIns"), _("Insert a phoneme before the selected word") );
			// If we clicked the last word, show the append menu.
			if( _index == _pPhonList->GetNumWords() - 1 )
			{
				popupMenu.Append( MenuID_PhonemeWidgetAppendPhoneme, _("&Append phoneme"), _("Append a phoneme to the end of the phoneme list.") );
			}
			popupMenu.Append( MenuID_PhonemeWidgetGroupSelection,  _("&Rename word\tCtrl+G"), _("Rename the word") );
			popupMenu.Append( MenuID_PhonemeWidgetUngroupSelection, _("&Ungroup word\tCtrl+U"), _("Remove the selected word, leaving the phonemes") );
			popupMenu.AppendSeparator();
			popupMenu.Append( MenuID_PhonemeWidgetPlaySelection, _("P&lay word\tSpace"), _("Play the audio for this word") );
		}
		else
		{
			// Default behavior
			popupMenu.Append( MenuID_PhonemeWidgetAppendPhoneme, _("&Append phoneme"), _("Append a phoneme to the end of the phoneme list.") );
			popupMenu.Append( MenuID_PhonemeWidgetGroupSelection, _("&Group to word\tCtrl+G"), _("Group the selected phonemes to a word") );
			if( !_pPhonList->HasSelection() )
			{
				popupMenu.GetMenuItems()[popupMenu.GetMenuItemCount()-1]->Enable( FALSE );
			}
		}

		popupMenu.AppendSeparator();
		popupMenu.Append( MenuID_PhonemeWidgetResetView, _("Reset &view\tCtrl+R"), _("Reset the view to the entire phoneme list") );
		if( _pPhonList->HasSelection() )
		{
			popupMenu.Append( MenuID_PhonemeWidgetZoomSelection, _("&Zoom selection\tCtrl+W"), _("Zooms to the current selection") );
		}
		popupMenu.Append( MenuID_PhonemeWidgetReanalyzeSelection, _("Re&analyze selection\tCtrl+Q"), _("Reanalyzes the current selection") );
		PopupMenu( &popupMenu, event.GetPosition() );
	}
}

void FxPhonemeWidget::OnMiddleButtonDown( wxMouseEvent& FxUnused(event) )
{
	SetFocus();
	if( !HasCapture() )
	{
		CaptureMouse();
	}
	Refresh(FALSE);
}

void FxPhonemeWidget::OnMiddleButtonUp( wxMouseEvent& FxUnused(event) )
{
	if( HasCapture() )
	{
		ReleaseMouse();
	}
}

void FxPhonemeWidget::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxStopWatch watch;

	wxPaintDC front( this );

	wxBrush backgroundBrush( _phonClassInfo[PHONCLASS_BACKGROUND].colour, wxSOLID );
	wxPen   linePen( *wxBLACK, 1, wxSOLID );

	wxRect clientRect = GetClientRect();

	// Set up the backbuffer
	wxMemoryDC dc;
	wxBitmap backbuffer( front.GetSize().GetWidth(), front.GetSize().GetHeight() );
	dc.SelectObject( backbuffer );
	dc.SetBackground( backgroundBrush  );
	dc.Clear();

	if( _pPhonList )
	{
		// Do the drawing.
		dc.SetPen( linePen );

		_ySplit = clientRect.GetHeight() / 2;

		FxInt32 timeHintPixel = FxTimeManager::TimeToCoord( _currentTime, clientRect );

		for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
		{
			FxReal phonStart = _pPhonList->GetPhonemeStartTime(i);
			FxReal phonEnd   = _pPhonList->GetPhonemeEndTime(i);
			if( phonEnd >= _minTime && phonStart <= _maxTime )
			{
				FxInt32 left = FxTimeManager::TimeToCoord(phonStart, clientRect );
				FxInt32 right= FxTimeManager::TimeToCoord(phonEnd, clientRect );
				FxInt32 top = _phonsOnTop ? clientRect.GetTop() : _ySplit;
				FxInt32 bottom = _phonsOnTop ? _ySplit : clientRect.GetBottom();
				const FxPhonExtendedInfo& info = FxGetPhonemeInfo(_pPhonList->GetPhonemeEnum( i ));
				wxString label;
				switch( FxStudioOptions::GetPhoneticAlphabet() )
				{
				case PA_SAMPA:
					label = info.sampa;
					break;
				case PA_IPA:
					label = info.ipa;
					break;
				default:
				case PA_DEFAULT:
					label = info.talkback;
				}

				// Silence should fill the bar
				FxBool phonInWord = _pPhonList->IsPhonemeInWord( i );
				if( (info.phoneme == PHON_SIL && !phonInWord ) || 
					( _ungroupedPhonsFillBar && !phonInWord ) )
				{
					top = clientRect.GetTop();
					bottom = clientRect.GetBottom();
				}

				FxPhoneme phon = _pPhonList->GetPhonemeEnum( i );
				wxColour brushColour = _phonClassInfo[FxGetPhonemeInfo(phon).phonClass].colour;

				// Select the correct brush
				if( _mouseAction != MOUSE_NONE &&
					_pPhonList->GetPhonemeDuration( i ) <= _pPhonList->GetMinPhonemeLength() + FX_REAL_EPSILON )
				{
					brushColour = _phonClassInfo[PHONCLASS_MINSIZED].colour;
				}
				if( _pPhonList->GetPhonemeSelected( i ) )
				{
					brushColour = _phonClassInfo[PHONCLASS_SELECTED].colour;
				}
				else if( left <= timeHintPixel && timeHintPixel < right )
				{
					brushColour = _phonClassInfo[PHONCLASS_CURRTIME].colour;
				}

				DrawLabeledRectangle( &dc, wxPoint( left, top ), wxPoint( right, bottom ),
					wxBrush( brushColour, wxSOLID ), label );
			}
		}
		for( FxSize i = 0; i < _pPhonList->GetNumWords(); ++i )
		{
			FxInt32 left = FxTimeManager::TimeToCoord(_pPhonList->GetWordStartTime( i ), clientRect );
			FxInt32 right   = FxTimeManager::TimeToCoord(_pPhonList->GetWordEndTime( i ), clientRect );
			FxInt32 top = !_phonsOnTop ? clientRect.GetTop() : _ySplit;
			FxInt32 bottom = !_phonsOnTop ? _ySplit : clientRect.GetBottom();
			wxString label( _pPhonList->GetWordText( i ) );

			wxColour brushColour = _phonClassInfo[PHONCLASS_WORD].colour;

			if( _pPhonList->GetWordSelected( i ) )
			{
				brushColour = _phonClassInfo[PHONCLASS_SELECTED].colour;
			}
			else if( left <= timeHintPixel && timeHintPixel < right )
			{
				brushColour = _phonClassInfo[PHONCLASS_CURRTIME].colour;
			}

			DrawLabeledRectangle( &dc, wxPoint( left, top ), wxPoint( right, bottom ),
				wxBrush( brushColour, wxSOLID ), label );
		}

		if( _currentTime != FxInvalidValue && 
			_minTime <= _currentTime && _currentTime <= _maxTime )
		{
			dc.SetPen( wxPen(wxColour(100, 100, 100), 1, wxDOT) );
			dc.DrawLine( wxPoint( timeHintPixel, clientRect.GetTop() ), wxPoint( timeHintPixel, clientRect.GetBottom() ) );
		}

		if( _hasFocus )
		{
			dc.SetPen( wxPen(FxStudioOptions::GetFocusedWidgetColour(), 1, wxSOLID) );
			dc.SetBrush( *wxTRANSPARENT_BRUSH );
			dc.DrawRectangle( GetClientRect() );
		}
	}
	// Swap to the front buffer
	front.Blit( front.GetLogicalOrigin(), front.GetSize(), &dc, dc.GetLogicalOrigin(), wxCOPY );

	// Clean up
	dc.SetFont( wxNullFont );

	lastPaintTime = watch.Time();
}

// Resets the view
void FxPhonemeWidget::OnResetView( wxCommandEvent& FxUnused(event) )
{
	if( _pPhonList )
	{
		FxTimeManager::RequestTimeChange( FxTimeManager::GetMinimumTime(), FxTimeManager::GetMaximumTime() );
	}
}

// Plays the selection.
void FxPhonemeWidget::OnPlaySelection( wxCommandEvent& FxUnused(event) )
{
	FxStudioAnimPlayer* animPlayer = FxStudioAnimPlayer::Instance();
	if( _pPhonList && animPlayer )
	{
		FxReal selectionStart = FX_REAL_MAX;
		FxReal selectionEnd   = FX_REAL_MIN;
		// Calculate the extents of the selection.
		for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
		{
			if( _pPhonList->GetPhonemeSelected( i ) )
			{
				selectionStart = FxMin( selectionStart, _pPhonList->GetPhonemeStartTime( i ) );
				selectionEnd   = FxMax( selectionEnd, _pPhonList->GetPhonemeEndTime( i ) );
			}
		}

		if( selectionEnd > selectionStart )
		{
			// Play the current selection range of audio.
			animPlayer->PlaySection(selectionStart, selectionEnd);
		}
		else if( selectionStart == FX_REAL_MAX && selectionEnd == FX_REAL_MIN )
		{
			// Play the full range of audio.
			animPlayer->Play(FxFalse);
		}
	}
}

// Deletes the selection
void FxPhonemeWidget::OnDeleteSelection( wxCommandEvent& FxUnused(event) )
{
	if( _pPhonList )
	{
		FlagNoUnload();
		FxVM::ExecuteCommand("phonList -p");
		_pPhonList->RemoveSelection();
		Refresh( FALSE );
		if( _pPhonList->IsDirty() )
		{
			if( _mediator )
			{
				_mediator->OnAnimPhonemeListChanged(this, _pAnim, _pPhonList);
			}
		}
	}
}

// Groups the selection
void FxPhonemeWidget::OnGroupSelection( wxCommandEvent& FxUnused(event) )
{
	if( _pPhonList && _pPhonList->HasSelection() )
	{
		FlagNoUnload();
		FxVM::ExecuteCommand("phonList -p");
		// Generate the default string, which is all the selected words appended
		// onto each other.
		wxString def = wxEmptyString;
		for( FxSize i = 0; i < _pPhonList->GetNumWords(); ++i )
		{
			if( _pPhonList->GetWordSelected( i ) )
			{
				def.Append( _pPhonList->GetWordText( i ) );
				def.Append( wxT(" ") );
			}
		}
		// Remove that last space if it's been appended.
		if( def != wxEmptyString )
		{
			def.RemoveLast();
		}

		//@todo If the string is still empty we could do something cool like
		//      scanning the current dictionary for a phonetic sequence matching the
		//      current selection and use the corresponding word as our default in 
		//      the message box.

		// Get the user's input
		wxString word = ::wxGetTextFromUser( _("Enter label:"), _("Input text"), def, this );
		if( word != wxEmptyString )
		{
			FxSize start = _pPhonList->GetNumPhonemes();
			FxSize end   = 0;
			for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
			{
				if( _pPhonList->GetPhonemeSelected( i ) )
				{
					start = FxMin( start, i );
					end   = FxMax( end, i );
				}
			}

			// If we're grouping to a word a subset of a larger word, ungroup the
			// larger word.
			FxSize startWord = FxInvalidIndex;
			FxSize endWord = FxInvalidIndex;
			if( _pPhonList->IsPhonemeInWord(start, &startWord) && 
				_pPhonList->IsPhonemeInWord(end, &endWord) && 
				startWord == endWord && 
				start > _pPhonList->GetWordStartPhoneme(startWord) &&
				end < _pPhonList->GetWordEndPhoneme(endWord) )
			{
				_pPhonList->Ungroup(_pPhonList->GetWordStartPhoneme(startWord),
								    _pPhonList->GetWordEndPhoneme(endWord) );
			}

			_pPhonList->GroupToWord( word, start, end );
			Refresh( FALSE );
		}
	}
}

// Ungroups the selection
void FxPhonemeWidget::OnUngroupSelection( wxCommandEvent& FxUnused(event) )
{
	if( _pPhonList )
	{
		FlagNoUnload();
		FxVM::ExecuteCommand("phonList -p");
		// Find the start and end of the phonetic selection
		FxSize start = _pPhonList->GetNumPhonemes();
		FxSize end   = 0;
		for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
		{
			if( _pPhonList->GetPhonemeSelected( i ) )
			{
				start = FxMin( start, i );
				end   = FxMax( end, i );
			}
		}
		// Ask the phoneme list to remove any words associated with those phones
		_pPhonList->Ungroup( start, end );
		Refresh( FALSE );
	}
}

// Responds to a change phoneme message
void FxPhonemeWidget::OnPhonemeMenu( wxCommandEvent& event )
{
	if( _pPhonList )
	{
		FlagNoUnload();
		FxVM::ExecuteCommand("phonList -p");
		_pPhonList->ChangeSelection(static_cast<FxPhoneme>(event.GetId() - PHONMENU_MODIFIER));
		Refresh( FALSE );
		if( _pPhonList->IsDirty() )
		{
			if( _mediator )
			{
				_mediator->OnAnimPhonemeListChanged(this, _pAnim, _pPhonList);
			}
		}
	}
}

void FxPhonemeWidget::OnQuickChangePhoneme( wxCommandEvent& FxUnused(event) )
{

	if( _pPhonList )
	{
		FxChangePhonemeDialog dialog(this);
		if( dialog.ShowModal() == wxID_OK )
		{
			FlagNoUnload();
			FxVM::ExecuteCommand("phonList -p");
			_pPhonList->ChangeSelection(dialog.selectedPhoneme);
			Refresh( FALSE );
			if( _pPhonList->IsDirty() )
			{
				if( _mediator )
				{
					_mediator->OnAnimPhonemeListChanged(this, _pAnim, _pPhonList);
				}
			}
		}
	}
}

void FxPhonemeWidget::OnInsertPhoneme( wxCommandEvent& FxUnused(event) )
{
	FxSize insertionIndex = FxInvalidIndex;
	if( _pPhonList  && _pPhonList->HasSelection() )
	{
		for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
		{
			if( _pPhonList->GetPhonemeSelected( i ) )
			{
				insertionIndex = i;
				break;
			}
		}
	}

	if( _pPhonList && insertionIndex != FxInvalidIndex)
	{
		FlagNoUnload();
		FxVM::ExecuteCommand("phonList -p");
		// Check the length of the phoneme.  If it's greater than twice the 
		// minimum phoneme duration, use split time, otherwise push the 
		// succeeding phonemes back to make room.
		FxPhonWordList::InsertionMethod insMethod = FxPhonWordList::PUSH_OUT;
		if( _pPhonList->GetPhonemeDuration( insertionIndex ) >= 
			2 * _pPhonList->GetMinPhonemeLength() )
		{
			insMethod = FxPhonWordList::SPLIT_TIME;
		}
		_pPhonList->InsertPhoneme( PHON_SIL, insertionIndex, insMethod );
		Refresh( FALSE );
		if( _pPhonList->IsDirty() )
		{
			if( _mediator )
			{
				_mediator->OnAnimPhonemeListChanged(this, _pAnim, _pPhonList);
			}
		}
	}
}

void FxPhonemeWidget::OnAppendPhoneme( wxCommandEvent& FxUnused(event) )
{
	if( _pPhonList )
	{
		FlagNoUnload();
		FxVM::ExecuteCommand("phonList -p");

		FxReal phonListEndTime = 0.0f;
		if( _pPhonList->GetNumPhonemes() > 0 )
		{
			phonListEndTime = _pPhonList->GetPhonemeEndTime(_pPhonList->GetNumPhonemes()-1);
		}
		_pPhonList->AppendPhoneme( PHON_SIL, phonListEndTime, phonListEndTime + 0.25f );

		Refresh( FALSE );
		if( _pPhonList->IsDirty() )
		{
			if( _mediator )
			{
				_mediator->OnAnimPhonemeListChanged(this, _pAnim, _pPhonList);
			}
		}
	}
}

// Displays the options menu
void FxPhonemeWidget::OnOptions( wxCommandEvent& FxUnused(event) )
{
	FxPhonemeWidgetOptionsDlg optionsDlg;
	optionsDlg.Create( this );
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( optionsDlg.ShowModal() == wxID_OK )
	{
		optionsDlg.SetOptions();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxPhonemeWidget::OnCut( wxCommandEvent& event )
{
	OnCopy( event );
	FlagNoUnload();
	FxVM::ExecuteCommand("phonList -p");
	_pPhonList->ObliterateSelection();
	Refresh( FALSE );
	DispatchPhonemesChanged();
}

void FxPhonemeWidget::OnCopy( wxCommandEvent& FxUnused(event) )
{
	_copyPhoneBuffer.Clear();
	_copyWordBuffer.Clear();
	for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
	{
		if( _pPhonList->GetPhonemeSelected(i) )
		{
			FxPhonInfo currPhon;
			currPhon.phoneme = _pPhonList->GetPhonemeEnum(i);
			currPhon.start   = _pPhonList->GetPhonemeStartTime(i);
			currPhon.end     = _pPhonList->GetPhonemeEndTime(i);
			_copyPhoneBuffer.PushBack( currPhon );
		}
	}
	FxSize startWord = FxInvalidIndex;
	FxSize startPhon = FxInvalidIndex;
	for( FxSize i = 0; i < _pPhonList->GetNumWords(); ++i )
	{
		if( _pPhonList->GetWordSelected(i) )
		{
			if( startWord == FxInvalidIndex )
			{
				// Find the phoneme of the start so we can get the correct offset
				startWord = i;
				startPhon = _pPhonList->GetWordStartPhoneme(startWord);
				while( startPhon > 0 && _pPhonList->GetPhonemeSelected(startPhon-1) )
				{
					startPhon--;
				}
			}
			FxWordInfo currWord;
			currWord.text  = _pPhonList->GetWordText(i);
			currWord.first = _pPhonList->GetWordStartPhoneme(i) - startPhon;
			currWord.last  = _pPhonList->GetWordEndPhoneme(i) - startPhon;
			_copyWordBuffer.PushBack( currWord );
		}
	}
}

void FxPhonemeWidget::OnPaste( wxCommandEvent& FxUnused(event) )
{
	FxSize start = FxInvalidIndex;
	for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
	{
		if( _pPhonList->GetPhonemeSelected(i) )
		{
			if( start == FxInvalidIndex )
			{
				start = i;
				break;
			}
		}
	}
	if( start == FxInvalidIndex )
	{
		start = 0;
	}
	_pPhonList->ClearSelection();
	if( start != FxInvalidIndex )
	{
		FlagNoUnload();
		FxVM::ExecuteCommand("phonList -p");
		for( FxInt32 i = _copyPhoneBuffer.Length()-1; i >= 0; --i )
		{
			FxReal phoneDuration = _copyPhoneBuffer[i].end - _copyPhoneBuffer[i].start;
			_pPhonList->InsertPhoneme( _copyPhoneBuffer[i].phoneme, phoneDuration, start );
		}
		for( FxSize i = 0; i < _copyWordBuffer.Length(); ++i )
		{
			FxSize startWord = FxInvalidIndex;
			FxSize endWord = FxInvalidIndex;
			if( _pPhonList->IsPhonemeInWord(_copyWordBuffer[i].first + start, &startWord) && 
				_pPhonList->IsPhonemeInWord(_copyWordBuffer[i].last + start, &endWord) && 
				startWord == endWord && 
				_copyWordBuffer[i].first + start > _pPhonList->GetWordStartPhoneme(startWord) &&
				_copyWordBuffer[i].last + start < _pPhonList->GetWordEndPhoneme(endWord) )
			{
				_pPhonList->Ungroup(_pPhonList->GetWordStartPhoneme(startWord),
					_pPhonList->GetWordEndPhoneme(endWord) );
			}

			_pPhonList->GroupToWord( _copyWordBuffer[i].text, 
				_copyWordBuffer[i].first + start, _copyWordBuffer[i].last + start );
		}
		DispatchPhonemesChanged();
	}
	Refresh( FALSE );
}

void FxPhonemeWidget::OnUndo( wxCommandEvent& FxUnused(event) )
{
	FxVM::ExecuteCommand("undo");
}

void FxPhonemeWidget::OnRedo( wxCommandEvent& FxUnused(event) )
{
	FxVM::ExecuteCommand("redo");
}

void FxPhonemeWidget::OnZoomSelection( wxCommandEvent& FxUnused(event) )
{
	FxReal selectionStart = FX_REAL_MAX;
	FxReal selectionEnd   = FX_REAL_MIN;

	// Calculate the extents of the selection.
	for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
	{
		if( _pPhonList->GetPhonemeSelected(i) )
		{
			selectionStart = FxMin(selectionStart, _pPhonList->GetPhonemeStartTime(i));
			selectionEnd   = FxMax(selectionEnd, _pPhonList->GetPhonemeEndTime(i));
		}
	}

	if( selectionEnd > selectionStart )
	{
		FxTimeManager::RequestTimeChange(selectionStart, selectionEnd);
	}
}

void FxPhonemeWidget::OnReanalyzeSelection( wxCommandEvent& FxUnused(event) )
{
	// Prevent re-entry into this function before processing is finished, which
	// seems to be possible by repeatedly using the keyboard shortcut after 
	// dismissing the dialog.
	static bool isBusy = FxFalse;
	if( _pPhonList && !isBusy )
	{
		isBusy = FxTrue;
		FxReal selectionStart = FX_REAL_MAX;
		FxReal selectionEnd   = FX_REAL_MIN;

		// Calculate the extents of the selection.
		for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
		{
			if( _pPhonList->GetPhonemeSelected(i) )
			{
				selectionStart = FxMin(selectionStart, _pPhonList->GetPhonemeStartTime(i));
				selectionEnd   = FxMax(selectionEnd, _pPhonList->GetPhonemeEndTime(i));
			}
		}

		if( selectionEnd > selectionStart )
		{
			FxReal duration = selectionEnd - selectionStart;
			if( duration > static_cast<FxReal>(*a_audiomax) )
			{
				wxString msg = wxString::Format(_("You cannot reanalyze more than %f seconds of audio.  Please select a smaller range."), static_cast<FxReal>(*a_audiomax));
				wxString caption = wxString::Format(_("%f second limit reached"), static_cast<FxReal>(*a_audiomax));
				FxMessageBox(msg, caption);
			}
			else if( duration < static_cast<FxReal>(*a_audiomin) )
			{
				wxString msg = wxString::Format(_("You cannot reanalyze less than %f seconds of audio.  Please select a larger range."), static_cast<FxReal>(*a_audiomin));
				wxString caption = wxString::Format(_("%f second limit reached"), static_cast<FxReal>(*a_audiomin));
				FxMessageBox(msg, caption);
			}
			else
			{
				if( _mediator )
				{
					FxReanalyzeSelectionDialog reanalyzeDialog;
					reanalyzeDialog.Initialize(_mediator, _pPhonList);

					// Figure out what text to initialize the reanalyze selection
					// dialog with.
					wxString selectedWords = wxEmptyString;
					for( FxSize i = 0; i < _pPhonList->GetNumWords(); ++i )
					{
						if( _pPhonList->GetWordSelected(i) )
						{
							selectedWords.Append(_pPhonList->GetWordText(i));
							selectedWords.Append(wxT(" "));
						}
					}
					// Remove that last space if it's been appended.
					if( selectedWords != wxEmptyString )
					{
						selectedWords.RemoveLast();
						// If there are selected words, use those.
						reanalyzeDialog.SetAnalysisText(FxWString(selectedWords.GetData()));
					}
					else if( _pAnim )
					{
						// Otherwise use the full analysis text from the user data.
						FxAnimUserData* pUserData = reinterpret_cast<FxAnimUserData*>(_pAnim->GetUserData());
						if( pUserData )
						{
							reanalyzeDialog.SetAnalysisText(pUserData->GetAnalysisText());
						}
					}

					reanalyzeDialog.Create(this);
					FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
					if( wxID_OK == reanalyzeDialog.ShowModal() )
					{
						_mediator->OnReanalyzeRange(this, _pAnim, selectionStart, selectionEnd, reanalyzeDialog.GetAnalysisText());
					}
					FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
				}
			}
		}
		isBusy = FxFalse;
	}
}

void FxPhonemeWidget::OnGetFocus( wxFocusEvent& FxUnused(event) )
{
	_hasFocus = FxTrue;
	Refresh(FALSE);
}

void FxPhonemeWidget::OnLoseFocus( wxFocusEvent& FxUnused(event) )
{
	_hasFocus = FxFalse;
	Refresh(FALSE);
}

void FxPhonemeWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Animation Tab"));
}

// Draws a labeled rectangle
void FxPhonemeWidget::DrawLabeledRectangle( wxDC* pDC, const wxPoint& leftTop, 
						  const wxPoint& rightBottom, const wxBrush& brush, 
						  const wxString& label )
{
	wxPoint center( (rightBottom.x + leftTop.x) / 2, 
					(rightBottom.y + leftTop.y) / 2 );

	wxRect toDraw(leftTop, rightBottom);

	// Select the font
	wxFont font( FxMax( 2.0f, FxMin( (toDraw.GetWidth() * 0.75f) / label.Length(), GetSize().GetHeight() / 4.f )), wxDEFAULT, wxNORMAL, wxBOLD, FALSE, FxStudioOptions::GetUnicodeFontName() );
	pDC->SetFont( font );

	FxInt32 xSize, ySize;
	pDC->GetTextExtent( label, &xSize, &ySize );

	pDC->SetBrush( brush );
	pDC->DrawRectangle( toDraw );
	wxPoint textPos( center.x - (xSize / 2), center.y - (ySize / 2 ) );

	pDC->SetTextForeground( _phonClassInfo[PHONCLASS_TEXTBACKGROUND].colour );
	pDC->DrawText( label, textPos.x + 1, textPos.y + 1 );
	pDC->SetTextForeground( _phonClassInfo[PHONCLASS_TEXTFOREGROUND].colour );
	pDC->DrawText( label, textPos );
}

void FxPhonemeWidget::Zoom( const wxPoint& zoomPoint, FxReal zoomFactor, 
						     eZoomDir zoomDir )
{
	if( zoomDir != ZOOM_NONE )
	{
		wxRect clientRect = GetClientRect();
		FxReal timeSpan = _maxTime - _minTime;
		FxReal time  = FxTimeManager::CoordToTime( zoomPoint.x, clientRect );
		FxReal timePct = (time - _minTime) / timeSpan;

		// Zoom in/out around the point by the 'zoom factor'
		FxReal newTimeSpan = (zoomDir == ZOOM_IN) 
			? (timeSpan * zoomFactor) : (timeSpan / zoomFactor);

		// Recalculate the minimum and maximum times
		FxReal tempMinTime = time - ( newTimeSpan * timePct );
		FxReal tempMaxTime = time + ( newTimeSpan * (1 - timePct ) );
		FxTimeManager::RequestTimeChange( tempMinTime, tempMaxTime );
	}
}

FxBool FxPhonemeWidget::HitTestPhonBoundaries( wxPoint point, FxSize* phonIndex, eSide* phonSide  )
{
	wxRect clientRect = GetClientRect();
	static const FxInt32 tolerance = 4;
	for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
	{
		if( (_phonsOnTop && point.y < _ySplit) || (!_phonsOnTop && point.y > _ySplit) )
		{
			FxInt32 x = FxTimeManager::TimeToCoord( _pPhonList->GetPhonemeEndTime( i ), clientRect );
			if( x - tolerance <= point.x && point.x <= x )
			{
				if( phonIndex )
				{
					*phonIndex = i;
				}
				if( phonSide )
				{
					*phonSide = END;
				}
				return FxTrue;
			}
			x = FxTimeManager::TimeToCoord( _pPhonList->GetPhonemeStartTime( i ), clientRect );
			if( x <= point.x && point.x <= x + tolerance )
			{
				if( phonIndex )
				{
					*phonIndex = i;
				}
				if( phonSide )
				{
					*phonSide = START;
				}
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxPhonemeWidget::HitTestPhonemes( wxPoint point, FxSize* phonIndex )
{
	wxRect clientRect = GetClientRect();
	for( FxSize i = 0; i < _pPhonList->GetNumPhonemes(); ++i )
	{
		FxReal start   = _pPhonList->GetPhonemeStartTime( i );
		FxReal end     = _pPhonList->GetPhonemeEndTime( i );
		FxInt32 xStart = FxTimeManager::TimeToCoord( start, clientRect );
		FxInt32 xEnd   = FxTimeManager::TimeToCoord( end, clientRect );
		if( _pPhonList->GetPhonemeEnum( i ) == PHON_SIL || 
			(_phonsOnTop && point.y < _ySplit) || 
			(!_phonsOnTop && point.y > _ySplit) )
		{
			if( xStart <= point.x && point.x <= xEnd )
			{
				if( phonIndex )
				{
					*phonIndex = i;
				}
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxPhonemeWidget::HitTestWordBoundaries( wxPoint point, FxSize* wordIndex, eSide* wordSide )
{
	wxRect clientRect = GetClientRect();
	static const FxInt32 tolerance = 4;
	for( FxSize i = 0; i < _pPhonList->GetNumWords(); ++i )
	{
		if( (!_phonsOnTop && point.y < _ySplit) || (_phonsOnTop && point.y > _ySplit) )
		{
			FxInt32 x = FxTimeManager::TimeToCoord( _pPhonList->GetWordStartTime( i ), clientRect );
			if( x - tolerance <= point.x && point.x <= x + tolerance )
			{
				if( wordIndex )
				{
					*wordIndex = i;
				}
				if( wordSide )
				{
					*wordSide = START;
				}
				return FxTrue;
			}
			x = FxTimeManager::TimeToCoord( _pPhonList->GetWordEndTime( i ), clientRect );
			if( x - tolerance <= point.x && point.x <= x + tolerance )
			{
				if( wordIndex )
				{
					*wordIndex = i;
				}
				if( wordSide )
				{
					*wordSide = END;
				}
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

FxBool FxPhonemeWidget::HitTestWords( wxPoint point, FxSize* wordIndex )
{
	wxRect clientRect = GetClientRect();
	for( FxSize i = 0; i < _pPhonList->GetNumWords(); ++i )
	{
		FxReal start   = _pPhonList->GetWordStartTime( i );
		FxReal end     = _pPhonList->GetWordEndTime( i );
		FxInt32 xStart = FxTimeManager::TimeToCoord( start, clientRect );
		FxInt32 xEnd   = FxTimeManager::TimeToCoord( end, clientRect );
		if( (!_phonsOnTop && point.y < _ySplit) || (_phonsOnTop && point.y > _ySplit) )
		{
			if( xStart <= point.x && point.x <= xEnd )
			{
				if( wordIndex )
				{
					*wordIndex = i;
				}
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

#define MAKE_PHONCLASSINFO( phonclass, label, col ) \
	FxPhonemeClassInfo phonclass##Info((phonclass), (label), (col), (col));\
	_phonClassInfo.PushBack( phonclass##Info );

#define DEFAULT_NONE_COLOUR wxColour( 255, 255, 255 )
#define DEFAULT_VOWEL_COLOUR wxColour( 100, 150, 220 )
#define DEFAULT_DIPHTHONG_COLOUR wxColour( 100, 150, 220 )
#define DEFAULT_FRICATIVE_COLOUR wxColour( 100, 150, 220 )
#define DEFAULT_NASAL_COLOUR wxColour( 100, 150, 220 )
#define DEFAULT_APPROXIMATE_COLOUR wxColour( 100, 150, 220 )
#define DEFAULT_PLOSIVE_COLOUR wxColour( 100, 150, 220 )
#define DEFAULT_TRILL_COLOUR wxColour( 100, 150, 220 )
#define DEFAULT_SILENCE_COLOUR wxColour( 75, 120, 180 )
#define DEFAULT_SELECTED_COLOUR wxColour( 230, 220, 50 )
#define DEFAULT_MINSIZED_COLOUR wxColour( 255, 0, 0 )
#define DEFAULT_WORD_COLOUR wxColour( 68, 105, 168 )
#define DEFAULT_TEXTFOREGROUND_COLOUR wxColour( 245, 245, 245 )
#define DEFAULT_TEXTBACKGROUND_COLOUR wxColour( 0, 0, 0 )
#define DEFAULT_BACKGROUND_COLOUR wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)
#define DEFAULT_CURRTIME_COLOUR wxColour( 230, 164, 0 )

void FxPhonemeWidget::Initialize()
{
	MAKE_PHONCLASSINFO( PHONCLASS_NONE, _("None"), DEFAULT_NONE_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_VOWEL, _("Vowel"), DEFAULT_VOWEL_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_DIPHTHONG, _("Diphthong"), DEFAULT_DIPHTHONG_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_FRICATIVE, _("Fricative"), DEFAULT_FRICATIVE_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_NASAL, _("Nasal"), DEFAULT_NASAL_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_APPROXIMATE, _("Approximant or Lateral"), DEFAULT_APPROXIMATE_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_PLOSIVE, _("Plosive or Affricate"), DEFAULT_PLOSIVE_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_TRILL, _("Trill"), DEFAULT_TRILL_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_SILENCE, _("Silence"), DEFAULT_SILENCE_COLOUR );

	MAKE_PHONCLASSINFO( PHONCLASS_SELECTED, _("Selected phoneme"), DEFAULT_SELECTED_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_MINSIZED, _("Minimum-sized phoneme"), DEFAULT_MINSIZED_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_WORD, _("Word"), DEFAULT_WORD_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_TEXTFOREGROUND, _("Text Foreground"), DEFAULT_TEXTFOREGROUND_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_TEXTBACKGROUND, _("Text Background"), DEFAULT_TEXTBACKGROUND_COLOUR);
	MAKE_PHONCLASSINFO( PHONCLASS_BACKGROUND, _("Background"), DEFAULT_BACKGROUND_COLOUR );
	MAKE_PHONCLASSINFO( PHONCLASS_CURRTIME, _("Current Time Highlight"), DEFAULT_CURRTIME_COLOUR );
}

void FxPhonemeWidget::DispatchPhonemesChanged()
{
	if( _pPhonList && _pPhonList->IsDirty() )
	{
		if( _mediator )
		{
			_mediator->OnAnimPhonemeListChanged(this, _pAnim, _pPhonList);
		}
	}
}

void FxPhonemeWidget::FlagNoUnload()
{
	if( _pAnim && _pAnim->GetUserData() )
	{
		reinterpret_cast<FxAnimUserData*>(_pAnim->GetUserData())->SetIsSafeToUnload(FxFalse);
	}
}

//------------------------------------------------------------------------------
// Options dialog
//------------------------------------------------------------------------------

WX_IMPLEMENT_DYNAMIC_CLASS( FxPhonemeWidgetOptionsDlg, wxDialog )

BEGIN_EVENT_TABLE( FxPhonemeWidgetOptionsDlg, wxDialog )
	EVT_BUTTON( ControlID_PhonemeWidgetApplyButton, FxPhonemeWidgetOptionsDlg::OnApply )
	EVT_BUTTON( ControlID_PhonemeWidgetRestoreDefaultButton, FxPhonemeWidgetOptionsDlg::OnRestoreDefault )

	EVT_RADIOBOX( ControlID_PhonemeWidgetPhoneticAlphabetRadio, FxPhonemeWidgetOptionsDlg::OnDirty )
	EVT_CHECKBOX( ControlID_PhonemeWidgetPhonemesFillBarCheck, FxPhonemeWidgetOptionsDlg::OnDirty )

	EVT_LISTBOX( ControlID_PhonemeWidgetColorsListBox, FxPhonemeWidgetOptionsDlg::OnColourListChange )
	EVT_BUTTON( ControlID_PhonemeWidgetChangeColorButton, FxPhonemeWidgetOptionsDlg::OnColourChange )
END_EVENT_TABLE()

FxPhonemeWidgetOptionsDlg::FxPhonemeWidgetOptionsDlg()
	: _phonemeWidget( NULL )
{
}

FxPhonemeWidgetOptionsDlg::FxPhonemeWidgetOptionsDlg( wxWindow* parent, 
			wxWindowID id, const wxString& caption,	const wxPoint& pos, 
			const wxSize& size, long style)
	: _phonemeWidget( NULL )
{
	Create( parent, id, caption, pos, size, style );
}

// Destructor
FxPhonemeWidgetOptionsDlg::~FxPhonemeWidgetOptionsDlg()
{
	delete[] _phonClassColours;
}

// Creation.
bool FxPhonemeWidgetOptionsDlg::Create( wxWindow* parent, wxWindowID id , 
			const wxString& caption, const wxPoint& pos, const wxSize& size, 
			long style )
{
	_phonemeWidget = static_cast<FxPhonemeWidget*>( parent );

	GetOptionsFromParent();

	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create( parent, id, caption, pos, size, style );

	SetAffirmativeId(ControlID_PhonemeWidgetOptionsDlgOkButton);
	SetEscapeId(ControlID_PhonemeWidgetCancelButton);

	CreateControls();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return TRUE;
}

// Creates the controls and sizers.
void FxPhonemeWidgetOptionsDlg::CreateControls()
{
	wxBoxSizer* boxSizerV = new wxBoxSizer( wxVERTICAL );
	this->SetSizer( boxSizerV );
	this->SetAutoLayout( TRUE );

	// Create the color selection component
	wxBoxSizer* boxSizerColours = new wxBoxSizer( wxHORIZONTAL );
	boxSizerV->Add( boxSizerColours, 0, wxALIGN_RIGHT|wxALL, 0 );

	FxInt32 numColours = _phonemeWidget->_phonClassInfo.Length() - 1;
	wxString* colourChoices = new wxString[numColours];
	for( FxSize i = 0; i < static_cast<FxSize>(numColours); ++i )
	{
		colourChoices[i] = _phonemeWidget->_phonClassInfo[i+1].label;
	}
	_colourList = new wxListBox( this, ControlID_PhonemeWidgetColorsListBox, wxDefaultPosition,
		wxDefaultSize, numColours, colourChoices );
	boxSizerColours->Add( _colourList, 0, wxALIGN_LEFT|wxALL, 5 );
	_colourList->SetSelection(0);

	wxBoxSizer* smallVert = new wxBoxSizer( wxVERTICAL );
	boxSizerColours->Add( smallVert, 0, wxALIGN_LEFT|wxALL, 5 );

	wxBitmap previewBmp( wxNullBitmap );
	_colourPreview = new wxStaticBitmap( this, ControlID_PhonemeWidgetColorSampleStaticBmp,
		previewBmp, wxDefaultPosition, wxSize( 92, 20 ), wxBORDER_SIMPLE );
	smallVert->Add( _colourPreview, 0, wxALIGN_LEFT|wxALL, 5 );
	_colourPreview->SetBackgroundColour( *wxRED );
	wxButton* changeColourBtn = new wxButton( this, ControlID_PhonemeWidgetChangeColorButton,
		_("Change...") );
	smallVert->Add( changeColourBtn, 0, wxALIGN_LEFT|wxALL, 5 );

	// Create the check box
	wxCheckBox* phonsFillBar = new wxCheckBox( this, ControlID_PhonemeWidgetPhonemesFillBarCheck, 
		_("Ungrouped phonemes fill the bar"), wxDefaultPosition, wxDefaultSize, 
		0, wxGenericValidator( &_ungroupedPhonsFillBar ) );
	boxSizerV->Add( phonsFillBar, 0, wxALIGN_LEFT|wxALL, 5 );


	wxButton* restoreDefaults = new wxButton( this, ControlID_PhonemeWidgetRestoreDefaultButton,
		_("Restore Defaults") );
	boxSizerV->Add( restoreDefaults, 0, wxGROW|wxALL, 5 );

	// Create the buttons at the bottom
	wxBoxSizer* boxSizerH = new wxBoxSizer( wxHORIZONTAL );
	boxSizerV->Add(boxSizerH, 0, wxGROW|wxALL, 5);

	wxButton* okButton = new wxButton( this, ControlID_PhonemeWidgetOptionsDlgOkButton, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	okButton->SetDefault();
	boxSizerH->Add(okButton, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* cancelButton = new wxButton( this, ControlID_PhonemeWidgetCancelButton, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	boxSizerH->Add(cancelButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
	_applyButton = new wxButton( this, ControlID_PhonemeWidgetApplyButton, _("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	_applyButton->Enable(false);
	boxSizerH->Add(_applyButton, 0, wxALIGN_RIGHT|wxALL, 5);

	wxCommandEvent temp;
	OnColourListChange( temp );
	Refresh( FALSE );
}

// Should we show tooltips?
FxBool FxPhonemeWidgetOptionsDlg::ShowToolTips()
{
	return FxTrue;
}

// Updates the phoneme widget with the current options.
void FxPhonemeWidgetOptionsDlg::SetOptions()
{
	for( FxSize i = 1; i < _phonemeWidget->_phonClassInfo.Length(); ++i )
	{
		_phonemeWidget->_phonClassInfo[i].colour = _phonClassColours[i-1];
	}

	_phonemeWidget->_ungroupedPhonsFillBar = _ungroupedPhonsFillBar;

	_phonemeWidget->Refresh( FALSE );
}

// Called whenever an action "dirties" the options
void FxPhonemeWidgetOptionsDlg::OnDirty( wxCommandEvent& event )
{
	_applyButton->Enable( TRUE );
	event.Skip();
}

// Called when the user clicks the apply button
void FxPhonemeWidgetOptionsDlg::OnApply( wxCommandEvent& FxUnused(event) )
{
	Validate();
	TransferDataFromWindow();
	SetOptions();
	_applyButton->Enable(false);
}

// Called when the user clicks the restore defaults button
void FxPhonemeWidgetOptionsDlg::OnRestoreDefault( wxCommandEvent& FxUnused(event) )
{
	if( _phonemeWidget )
	{
		_ungroupedPhonsFillBar = FxFalse;

		FxSize numColours = _phonemeWidget->_phonClassInfo.Length() - 1;
		for( FxSize i = 0; i < numColours; ++i )
		{
			_phonClassColours[i] = _phonemeWidget->_phonClassInfo[i+1].defaultColour;
		}
	}
	TransferDataToWindow();
	// Force a refresh of the colour swatch.
	wxCommandEvent temp;
	OnColourListChange(temp);
	Refresh(FALSE);
}

// Called when the color list box changes
void FxPhonemeWidgetOptionsDlg::OnColourListChange( wxCommandEvent& FxUnused(event) )
{
	FxSize index = _colourList->GetSelection();
	_colourPreview->SetBackgroundColour( _phonClassColours[index] );
	_colourPreview->Refresh();
}

// Called when the user clicks the "Change..." button to change a colour
void FxPhonemeWidgetOptionsDlg::OnColourChange( wxCommandEvent& FxUnused(event) )
{
	FxSize index = _colourList->GetSelection();
	wxColourDialog colorPickerDialog( this );
	colorPickerDialog.GetColourData().SetChooseFull(true);
	colorPickerDialog.GetColourData().SetColour( _phonClassColours[index] );
	// Remember the user's custom colors.
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		colorPickerDialog.GetColourData().SetCustomColour(i, FxColourMap::GetCustomColours()[i]);
	}
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( colorPickerDialog.ShowModal() == wxID_OK )
	{
		_phonClassColours[index] = colorPickerDialog.GetColourData().GetColour();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		FxColourMap::GetCustomColours()[i] = colorPickerDialog.GetColourData().GetCustomColour(i);
	}
	_colourPreview->SetBackgroundColour( _phonClassColours[index] );
	_applyButton->Enable( TRUE );
	Refresh( FALSE );
}

// Initializes the options to the current values
void FxPhonemeWidgetOptionsDlg::GetOptionsFromParent()
{
	if( _phonemeWidget )
	{
		_ungroupedPhonsFillBar = _phonemeWidget->_ungroupedPhonsFillBar;

		FxSize numColours = _phonemeWidget->_phonClassInfo.Length() - 1;
		_phonClassColours = new wxColour[ numColours ];
		for( FxSize i = 0; i < numColours; ++i )
		{
			_phonClassColours[i] = _phonemeWidget->_phonClassInfo[i+1].colour;
		}
	}
}

//------------------------------------------------------------------------------
// Phoneme change dialog.
//------------------------------------------------------------------------------
WX_IMPLEMENT_DYNAMIC_CLASS(FxChangePhonemeDialog, wxDialog)

BEGIN_EVENT_TABLE(FxChangePhonemeDialog, wxDialog)
	EVT_LISTBOX(ControlID_PhonemeWidgetQuickChangePhonDlgPhonemeListBox, FxChangePhonemeDialog::OnSelChange)
	EVT_LISTBOX_DCLICK(ControlID_PhonemeWidgetQuickChangePhonDlgPhonemeListBox, FxChangePhonemeDialog::OnOK)
	EVT_BUTTON(ControlID_PhonemeWidgetQuickChangePhonDlgOKButton, FxChangePhonemeDialog::OnOK)
END_EVENT_TABLE()

FxChangePhonemeDialog::FxChangePhonemeDialog(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, _("Change phoneme"), wxDefaultPosition, wxSize(175, 500))
{

	wxArrayString choices;
	FxPhoneticAlphabet phonAlphabet = FxStudioOptions::GetPhoneticAlphabet();
	for( FxSize i = PHON_FIRST; i < PHON_LAST; ++i )
	{
		wxString label;
		switch( phonAlphabet )
		{
		case PA_IPA:
			label = FxGetPhonemeInfo(i).ipa;
			break;
		case PA_SAMPA:
			label = FxGetPhonemeInfo(i).sampa;
			break;
		default:
		case PA_DEFAULT:
			label = FxGetPhonemeInfo(i).talkback;
			break;
		};

		choices.Add(label);
	}

	wxFont unicodeFont( 8.0f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode") );
	SetSizer(new wxBoxSizer(wxVERTICAL));
	_phonemeList = new wxListBox(this, ControlID_PhonemeWidgetQuickChangePhonDlgPhonemeListBox, wxDefaultPosition, wxDefaultSize, choices, wxLB_SORT);
	_phonemeList->SetFont(unicodeFont);
	GetSizer()->Add(_phonemeList, 1, wxGROW|wxALL, 5);
	_sampleWord = new wxStaticText(this, wxID_ANY, _(""));
	GetSizer()->Add(_sampleWord, 0, wxGROW|wxALL, 5);

	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	wxButton* ok = new wxButton(this, ControlID_PhonemeWidgetQuickChangePhonDlgOKButton, _("OK"));
	ok->SetDefault();
	wxButton* cancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
	row->Add(ok, 0, wxALL, 5);
	row->Add(cancel, 0, wxALL, 5);
	GetSizer()->Add(row);
}

void FxChangePhonemeDialog::OnSelChange(wxCommandEvent& FxUnused(event))
{
	FxPhoneticAlphabet phonAlphabet = FxStudioOptions::GetPhoneticAlphabet();

	FxInt32 sel = _phonemeList->GetSelection();
	wxString selStr = _phonemeList->GetString(sel);
	for( FxSize i = PHON_FIRST; i < PHON_LAST; ++i )
	{
		wxString label;
		switch( phonAlphabet )
		{
		case PA_IPA:
			label = FxGetPhonemeInfo(i).ipa;
			break;
		case PA_SAMPA:
			label = FxGetPhonemeInfo(i).sampa;
			break;
		default:
		case PA_DEFAULT:
			label = FxGetPhonemeInfo(i).talkback;
			break;
		};

		if( label == selStr )
		{
			selectedPhoneme = static_cast<FxPhoneme>(i);
			_sampleWord->SetLabel(FxGetPhonemeInfo(i).sampleWords);
			break;
		}
	}
}

void FxChangePhonemeDialog::OnOK(wxCommandEvent& FxUnused(event))
{
	EndDialog(wxID_OK);
}

} // namespace Face

} // namespace OC3Ent
