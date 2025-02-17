//------------------------------------------------------------------------------
// A curve widget, allowing editing of curves in an FxAnim.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxConsole.h"
#include "FxCurveWidget.h"
#include "FxAnim.h"
#include "FxVec2.h"
#include "FxMath.h"
#include "FxValidators.h"
#include "FxCurveUserData.h"
#include "FxStudioOptions.h"
#include "FxStudioApp.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/accel.h"
	#include "wx/toolbar.h"
#endif

namespace OC3Ent
{

namespace Face
{

static const FxInt32 FxGridSnapTolerance = 4;

FxCurveWidgetGrid::FxCurveWidgetGrid( FxCurveWidget* pWidget )
	: _pWidget( pWidget )
{
}

FxCurveWidgetGrid::~FxCurveWidgetGrid()
{
	_pWidget = 0;
}

void FxCurveWidgetGrid::SetWidget( FxCurveWidget* pWidget )
{
	_pWidget = pWidget;
}

static FxReal GetGridSpacing(FxInt32 gridNum)
{
	if(gridNum & 0x01) // Odd numbers
	{
		return FxPow( 10.f, 0.5f*((FxReal)(gridNum-1)) + 1.f );
	}
	else // Even numbers
	{
		return 0.5f * FxPow( 10.f, 0.5f*((FxReal)(gridNum)) + 1.f );
	}
}

void FxCurveWidgetGrid::Recalc()
{
	_values.Clear();
	FxTimeManager::GetTimes( _minTime, _maxTime );
	FxReal minValue, maxValue;
	_pWidget->GetGraphRange( minValue, maxValue );

	if( _minTime != FxInvalidValue && _maxTime != FxInvalidValue )
	{
		wxRect clientRect = _pWidget->GetWorkRect();

		// Calculate the value griding.
		FxReal  pixelsPerValue = static_cast<FxReal>(clientRect.GetHeight()) / (maxValue-minValue);
		FxInt32 minPixelsPerValueGrid = 25;
		FxReal  minGridSpacing = 0.001f;

		FxInt32 gridNum = 0;
		FxReal valueGridSpacing = minGridSpacing;
		while( valueGridSpacing * pixelsPerValue < minPixelsPerValueGrid )
		{
			valueGridSpacing = minGridSpacing * GetGridSpacing(gridNum);
			gridNum++;
		}
		_valueFormat = gridNum < 2 ? wxT("%3.3f") : wxT("%3.2f");

		FxInt32 valueNum = FxFloor(minValue/valueGridSpacing);
		while( valueNum*valueGridSpacing < maxValue )
		{
			_values.PushBack(valueNum*valueGridSpacing);
			valueNum++;
		}

		// Calculate the time griding.
		FxTimeManager::GetGridInfos(_timeGridInfos, clientRect, group);
	}
}

void FxCurveWidgetGrid::SetCurrTime(FxReal currTime)
{
	// Set the current time so we can snap to that, as well.
	_currTime = currTime;
}

void FxCurveWidgetGrid::DrawOn( wxDC* pDC )
{
	wxPen linePen( wxColour(170,170,170), 1, wxSOLID );
	pDC->SetPen( linePen );
	wxRect clientRect = _pWidget->GetWorkRect();

	pDC->SetFont( wxFont(8.f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode")) );

	FxInt32 x;
	FxInt32 xExtent = 0, yExtent = 0; 
	for( FxSize i = 0; i < _timeGridInfos.Length(); ++i )
	{
		x = _timeGridInfos[i].xValue;
		pDC->DrawLine( x, clientRect.GetTop(), x, clientRect.GetBottom() );
		pDC->GetTextExtent( _timeGridInfos[i].label, &xExtent, &yExtent );
		pDC->SetTextForeground(wxColour(0, 0, 0));
		pDC->DrawText( _timeGridInfos[i].label, x - (xExtent/2)+1, clientRect.GetBottom() - yExtent + 1 );
		pDC->SetTextForeground(wxColour(255,255,255));
		pDC->DrawText( _timeGridInfos[i].label, x - (xExtent/2), clientRect.GetBottom() - yExtent );
	}

	FxInt32 y;
	for( FxSize i = 0; i < _values.Length(); ++i )
	{
		y = _pWidget->ValueToCoord( _values[i], clientRect );
		if( y < clientRect.GetHeight() - yExtent + 1  + clientRect.GetTop() )
		{
			pDC->DrawLine( clientRect.GetLeft(), y, clientRect.GetRight(), y );
			wxString label = wxString::Format( _valueFormat, _values[i] );
			pDC->GetTextExtent( label, &xExtent, &yExtent );
			pDC->SetTextForeground(wxColour(0, 0, 0));
			pDC->DrawText( label, clientRect.GetLeft() + 11, y - (yExtent / 2) + 1 );
			pDC->SetTextForeground(wxColour(255,255,255));
			pDC->DrawText( label, clientRect.GetLeft() + 10, y - (yExtent / 2) );
		}
	}
	pDC->SetFont( wxNullFont );
	pDC->SetPen( wxNullPen );
}

FxReal FxCurveWidgetGrid::SnapTime( FxReal origTime, FxBool ignoreCurrentTime, FxAnimController* animController )
{
	wxRect clientRect = _pWidget->GetWorkRect();
	FxInt32 userCoord = FxTimeManager::TimeToCoord( origTime, clientRect, group );

	if( !ignoreCurrentTime )
	{
		// Try to snap to the current time.
		FxInt32 currTimeCoord = FxTimeManager::TimeToCoord(_currTime, clientRect, group);
		if( FxAbs(currTimeCoord - userCoord) < FxGridSnapTolerance )
		{
			return _currTime;
		}
	}


	if( animController )
	{
		FxSize numCurves = animController->GetNumCurves();
		FxSize numKeys = 0;
		FxReal keyTime = 0.0f;
		FxInt32 keyTimeCoord = 0;
		for( FxSize i = 0; i < numCurves; ++i )
		{
			if( animController->IsCurveVisible(i) && !animController->IsCurveOwnedByAnalysis(i) )
			{
				numKeys = animController->GetCurve(i)->GetNumKeys();
				for( FxSize j = 0; j < numKeys; ++j )
				{
					keyTime = animController->GetKeyTime(i, j);
					if( _minTime <= keyTime && keyTime <= _maxTime )
					{
						// Try to snap to the current time.
						keyTimeCoord = FxTimeManager::TimeToCoord(keyTime, clientRect, group);
						if( FxAbs(keyTimeCoord - userCoord) < FxGridSnapTolerance )
						{
							return keyTime;
						}
					}
				}
			}
		}
	}

	for( FxSize i = 0; i < _timeGridInfos.Length(); ++i )
	{
		if( _minTime <= _timeGridInfos[i].time && _timeGridInfos[i].time <= _maxTime )
		{
			FxInt32 gridCoord = FxTimeManager::TimeToCoord( _timeGridInfos[i].time, clientRect, group );
			if( FxAbs( gridCoord - userCoord ) < FxGridSnapTolerance )
			{
				return _timeGridInfos[i].time;
			}
		}
	}

	return origTime;
}

FxReal FxCurveWidgetGrid::SnapValue( FxReal origValue )
{
	wxRect clientRect = _pWidget->GetWorkRect();
	FxInt32 userCoord = _pWidget->ValueToCoord( origValue, clientRect );
	for( FxSize i = 0; i < _values.Length(); ++i )
	{
		FxInt32 gridCoord = _pWidget->ValueToCoord( _values[i], clientRect );
		if( FxAbs( gridCoord - userCoord ) < FxGridSnapTolerance )
		{
			return _values[i];
		}
	}
	return origValue;
}

FxInt32 FxCurveWidgetGrid::SnapX( FxInt32 x )
{
	wxRect clientRect = _pWidget->GetWorkRect();
	for( FxSize i = 0; i < _timeGridInfos.Length(); ++i )
	{
		FxInt32 gridCoord = FxTimeManager::TimeToCoord( _timeGridInfos[i].time, clientRect, group );
		if( FxAbs( gridCoord - x ) < FxGridSnapTolerance )
		{
			return gridCoord;
		}
	}
	return x;
}

FxInt32 FxCurveWidgetGrid::SnapY( FxInt32 y )
{
	wxRect clientRect = _pWidget->GetWorkRect();
	for( FxSize i = 0; i < _values.Length(); ++i )
	{
		FxInt32 gridCoord = _pWidget->ValueToCoord( _values[i], clientRect );
		if( FxAbs( gridCoord - y ) < FxGridSnapTolerance )
		{
			return gridCoord;
		}
	}
	return y;
}

WX_IMPLEMENT_DYNAMIC_CLASS( FxCurveWidget, wxWindow )

BEGIN_EVENT_TABLE( FxCurveWidget, wxWindow )
	EVT_PAINT( FxCurveWidget::OnPaint )
	EVT_MOUSEWHEEL( FxCurveWidget::OnMouseWheel )
	EVT_LEFT_DOWN( FxCurveWidget::OnLeftButtonDown )
	EVT_LEFT_UP( FxCurveWidget::OnLeftButtonUp )
	EVT_MOTION( FxCurveWidget::OnMouseMove )
	EVT_RIGHT_DOWN( FxCurveWidget::OnRightButtonDown )
	EVT_RIGHT_UP( FxCurveWidget::OnRightButtonUp )
	EVT_MIDDLE_DOWN( FxCurveWidget::OnMiddleButtonDown )
	EVT_MIDDLE_UP( FxCurveWidget::OnMiddleButtonUp )

	EVT_KEY_DOWN( FxCurveWidget::OnKeyDown )

	EVT_SIZE( FxCurveWidget::OnResize )

	EVT_MENU( MenuID_CurveWidgetResetView, FxCurveWidget::OnResetView )
	EVT_MENU( MenuID_CurveWidgetSelectAll, FxCurveWidget::OnSelectAll )
	EVT_MENU( MenuID_CurveWidgetLockTangent, FxCurveWidget::OnLockTangent )
	EVT_MENU( MenuID_CurveWidgetUnlockTangent, FxCurveWidget::OnUnlockTangent )
	EVT_MENU( MenuID_CurveWidgetDeleteSelection, FxCurveWidget::OnDeleteSelection )
//	EVT_MENU( MenuID_CurveWidgetEditKey, FxCurveWidget::OnEditKey )
	EVT_MENU( MenuID_CurveWidgetResetDerivatives, FxCurveWidget::OnResetSlopes )
	EVT_MENU( MenuID_CurveWidgetAddKey, FxCurveWidget::OnAddKey )

	EVT_MENU( MenuID_CurveWidgetCopy, FxCurveWidget::OnCopy )
	EVT_MENU( MenuID_CurveWidgetCut, FxCurveWidget::OnCut )
	EVT_MENU( MenuID_CurveWidgetPaste, FxCurveWidget::OnPaste )

	EVT_SET_FOCUS( FxCurveWidget::OnGetFocus )
	EVT_KILL_FOCUS( FxCurveWidget::OnLoseFocus )

	EVT_BUTTON( ControlID_CurveWidgetToolbarCut, FxCurveWidget::OnCut )
	EVT_BUTTON( ControlID_CurveWidgetToolbarCopy, FxCurveWidget::OnCopy )
	EVT_BUTTON( ControlID_CurveWidgetToolbarPaste, FxCurveWidget::OnPaste )
	EVT_BUTTON( ControlID_CurveWidgetToolbarLockTangent, FxCurveWidget::OnLockTangent )
	EVT_BUTTON( ControlID_CurveWidgetToolbarUnlockTangent, FxCurveWidget::OnUnlockTangent )
	EVT_BUTTON( ControlID_CurveWidgetToolbarFitValue, FxCurveWidget::OnFitValue )
	EVT_BUTTON( ControlID_CurveWidgetToolbarFitTime, FxCurveWidget::OnFitTime )
	EVT_BUTTON( ControlID_CurveWidgetToolbarViewAllActive, FxCurveWidget::OnViewAllActiveCurves )
	EVT_BUTTON( ControlID_CurveWidgetToolbarPrevKey, FxCurveWidget::OnPrevKey )
	EVT_BUTTON( ControlID_CurveWidgetToolbarNextKey, FxCurveWidget::OnNextKey )

	EVT_TEXT_ENTER( ControlID_CurveWidgetToolbarTimeDisplay, FxCurveWidget::OnUpdateKeys )
	EVT_TEXT_ENTER( ControlID_CurveWidgetToolbarValueDisplay, FxCurveWidget::OnUpdateKeys )
	EVT_TEXT_ENTER( ControlID_CurveWidgetToolbarIncDerivDisplay, FxCurveWidget::OnUpdateKeys )
	EVT_TEXT_ENTER( ControlID_CurveWidgetToolbarOutDerivDisplay, FxCurveWidget::OnUpdateKeys )

	EVT_HELP_RANGE( wxID_ANY, wxID_HIGHEST, FxCurveWidget::OnHelp )

END_EVENT_TABLE()

FxArray< FxArray<FxAnimKey> > FxCurveWidget::_copyBuffer;

//#define TB_POS(x) wxPoint((x)*25 + 5, 3)
//#define TB_SIZE wxSize(20,20)
#define TB_POS(x) wxDefaultPosition
#define TB_SIZE wxSize(20,20)

FxCurveWidget::FxCurveWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: Super( parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN|wxTAB_TRAVERSAL )
	, FxWidget( mediator )
	, _mouseWheelZoomFactor(1.5f)
	, _zoomFactor(1.035f)
	, _editCurve( FxInvalidIndex )
	, _editKey( FxInvalidIndex )
	, _editTangent( TANGENT_NONE )
	, _pAnimController( NULL )
	, _pAnim( NULL )
	, _minTime( 0.0f )
	, _maxTime( 1.0f )
	, _minValue( -6.0f )
	, _maxValue( 6.0f )
	, _currentTime( FxInvalidValue )
	, _hasFocus( FxFalse )
	, _oneDeePan( FxFalse )
	, _oneDeePanDirection( DIR_UNCERTAIN )
	, _oneDeeTransformDirection( DIR_UNCERTAIN )
	, _hasCurveSelection( FxFalse )
	, _lastTime( 0.0f )
	, _lastValue( 0.0f )
{
	_grid.SetWidget( this );
	_grid.group = FxDefaultGroup;
	SetBackgroundColour( wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE) );
	FxTimeManager::RegisterSubscriber( this );
	FxTimeManager::RequestTimeChange( _minTime, _maxTime );
	
	_textTime = NULL;
	SetSizer(new wxBoxSizer(wxVERTICAL));
	wxSizer* row1 = new wxBoxSizer(wxHORIZONTAL);
	wxSizer* row2 = row1;//new wxBoxSizer(wxHORIZONTAL);
	SetAutoLayout(FxTrue);
	// Create the buttons.
	FxButton* cut = new FxButton(this, ControlID_CurveWidgetToolbarCut, _("X"), TB_POS(0), TB_SIZE);
	cut->MakeToolbar();
	cut->SetToolTip(_("Cut"));
	row1->Add(cut, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	FxButton* copy = new FxButton(this, ControlID_CurveWidgetToolbarCopy, _("C"), TB_POS(1), TB_SIZE);
	copy->MakeToolbar();
	copy->SetToolTip(_("Copy"));
	row1->Add(copy, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	FxButton* paste = new FxButton(this, ControlID_CurveWidgetToolbarPaste, _("V"), TB_POS(2), TB_SIZE);
	paste->MakeToolbar();
	paste->SetToolTip(_("Paste"));
	row1->Add(paste, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	FxButton* lockTangents = new FxButton(this, ControlID_CurveWidgetToolbarLockTangent, _("LT"), TB_POS(3), TB_SIZE);
	lockTangents->MakeToolbar();
	lockTangents->SetToolTip(_("Lock Tangents"));
	row1->Add(lockTangents, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	FxButton* unlockTangents = new FxButton(this, ControlID_CurveWidgetToolbarUnlockTangent, _("UT"), TB_POS(4), TB_SIZE);
	unlockTangents->MakeToolbar();
	unlockTangents->SetToolTip(_("Unlock Tangents"));
	row1->Add(unlockTangents, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	_buttonSnapValue = new FxButton(this, ControlID_CurveWidgetToolbarSnapValue, _("SV"), TB_POS(5), TB_SIZE);
	_buttonSnapValue->SetToolTip(_("Snap red key movement to value grid lines"));
	_buttonSnapValue->MakeToggle();
	_buttonSnapValue->SetValue(FxTrue);
	_buttonSnapValue->MakeToolbar();
	row1->Add(_buttonSnapValue, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	_buttonSnapTime = new FxButton(this, ControlID_CurveWidgetToolbarSnapTime, _("ST"), TB_POS(6), TB_SIZE);
	_buttonSnapTime->SetToolTip(_("Snap red key movement to time grid lines"));
	_buttonSnapTime->MakeToggle();
	_buttonSnapTime->SetValue(FxTrue);
	_buttonSnapTime->MakeToolbar();
	row1->Add(_buttonSnapTime, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	row1->AddSpacer(5);
	_buttonFitValue = new FxButton(this, ControlID_CurveWidgetToolbarFitValue, _("FV"), TB_POS(7), TB_SIZE);
	_buttonFitValue->SetToolTip(_("Fit curves in value axis"));
	_buttonFitValue->MakeToolbar();
	row2->Add(_buttonFitValue, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	_buttonFitTime = new FxButton(this, ControlID_CurveWidgetToolbarFitTime, _("FT"), TB_POS(8), TB_SIZE);
	_buttonFitTime->SetToolTip(_("Fit curves in time axis"));
	_buttonFitTime->MakeToolbar();
	row2->Add(_buttonFitTime, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	row2->AddSpacer(5);
	_buttonViewAllActive = new FxButton(this, ControlID_CurveWidgetToolbarViewAllActive, _("VA"), TB_POS(9), TB_SIZE);
	_buttonViewAllActive->SetToolTip(_("View all active curves"));
	_buttonViewAllActive->MakeToolbar();
	row2->Add(_buttonViewAllActive, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	row2->AddSpacer(5);
	_buttonPrevKey = new FxButton(this, ControlID_CurveWidgetToolbarPrevKey, _("PK"), TB_POS(10), TB_SIZE);
	_buttonPrevKey->SetToolTip(_("Previous key"));
	_buttonPrevKey->MakeToolbar();
	row2->Add(_buttonPrevKey, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);
	_buttonNextKey = new FxButton(this, ControlID_CurveWidgetToolbarNextKey, _("NK"), TB_POS(10), TB_SIZE);
	_buttonNextKey->SetToolTip(_("Next key"));
	_buttonNextKey->MakeToolbar();
	row2->Add(_buttonNextKey, 0, wxTOP|wxBOTTOM|wxALIGN_LEFT, 3);

	_toolbarButtons.Reserve(10);
	_toolbarButtons.PushBack(cut);
	_toolbarButtons.PushBack(copy);
	_toolbarButtons.PushBack(paste);
	_toolbarButtons.PushBack(lockTangents);
	_toolbarButtons.PushBack(unlockTangents);
	_toolbarButtons.PushBack(_buttonSnapValue);
	_toolbarButtons.PushBack(_buttonSnapTime);
	_toolbarButtons.PushBack(_buttonFitValue);
	_toolbarButtons.PushBack(_buttonFitTime);
	_toolbarButtons.PushBack(_buttonViewAllActive);
	_toolbarButtons.PushBack(_buttonPrevKey);
	_toolbarButtons.PushBack(_buttonNextKey);

	row1->AddStretchSpacer();
	//row2->AddStretchSpacer();

	_textTime = new wxTextCtrl(this, ControlID_CurveWidgetToolbarTimeDisplay, wxEmptyString, TB_POS(9), wxSize(45, 20), 0, FxTimeValidator(&_keyeditTime));
	_textTime->SetToolTip(_("Time"));
	_textTime->Enable(FxFalse);
	row1->Add(_textTime, 0, wxALL|wxALIGN_RIGHT, 3);
	_textValue = new wxTextCtrl(this, ControlID_CurveWidgetToolbarValueDisplay, wxEmptyString, TB_POS(10), wxSize(45, 20), 0, FxRealValidator(&_keyeditValue));
	_textValue->SetToolTip(_("Value"));
	_textValue->Enable(FxFalse);
	row1->Add(_textValue, 0, wxALL|wxALIGN_RIGHT, 3);
	_textIncDeriv = new wxTextCtrl(this, ControlID_CurveWidgetToolbarIncDerivDisplay, wxEmptyString, TB_POS(11), wxSize(45,20), 0, FxRealValidator(&_keyeditIncDeriv));
	_textIncDeriv->SetToolTip(_("Incoming Deriviative"));
	_textIncDeriv->Enable(FxFalse);
	row2->Add(_textIncDeriv, 0, wxALL|wxALIGN_RIGHT, 3);
	_textOutDeriv = new wxTextCtrl(this, ControlID_CurveWidgetToolbarOutDerivDisplay, wxEmptyString, TB_POS(12), wxSize(45,20), 0, FxRealValidator(&_keyeditOutDeriv));
	_textOutDeriv->SetToolTip(_("Outgoing Deriviative"));
	_textOutDeriv->Enable(FxFalse);
	row2->Add(_textOutDeriv, 0, wxALL|wxALIGN_RIGHT, 3);

	GetSizer()->Add(row1, 0, wxLEFT|wxRIGHT|wxEXPAND, 3);
	//GetSizer()->Add(row2, 0, wxLEFT|wxRIGHT|wxEXPAND, 3);
	Layout();

	SetToolbarState();

	static const int numEntries = 9;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set( wxACCEL_CTRL, (FxInt32)'R', MenuID_CurveWidgetResetView );
	entries[1].Set( wxACCEL_CTRL, (FxInt32)'A', MenuID_CurveWidgetSelectAll );
	entries[2].Set( wxACCEL_CTRL, (FxInt32)'E', MenuID_CurveWidgetEditKey );
	entries[3].Set( wxACCEL_NORMAL, WXK_DELETE,  MenuID_CurveWidgetDeleteSelection );
	entries[4].Set( wxACCEL_NORMAL, WXK_INSERT,  MenuID_CurveWidgetAddKey );
	entries[5].Set( wxACCEL_CTRL, (FxInt32)'C', MenuID_CurveWidgetCopy );
	entries[6].Set( wxACCEL_CTRL, (FxInt32)'X', MenuID_CurveWidgetCut );
	entries[7].Set( wxACCEL_CTRL, (FxInt32)'V', MenuID_CurveWidgetPaste );
	entries[8].Set( wxACCEL_CTRL, (FxInt32)'D', MenuID_CurveWidgetResetDerivatives );
	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator( numEntries, entries );
	SetAcceleratorTable( accelerator );

	Initialize();
}

FxCurveWidget::~FxCurveWidget()
{
}

void FxCurveWidget::SetAnimControllerPointer( FxAnimController* pAnimController )
{
	_pAnimController = pAnimController;
}

void FxCurveWidget::SetAnimPointer( FxAnim* pAnim )
{
	if( _pAnim != pAnim )
	{
		_pAnim = pAnim;
		FxReal startTime = 0.0f;
		FxReal endTime   = 0.0f;
		if( _pAnimController )
		{
			_pAnimController->GetAnimationTime(startTime, endTime);
		}
		FxTimeManager::RequestTimeChange( startTime, endTime );
		_maxValue = ((_maxTime - _minTime) / 2.0f);
		_minValue = -_maxValue;
		_grid.Recalc();
		Refresh( FALSE );
	}
}

void FxCurveWidget::OnNotifyTimeChange()
{
	FxTimeManager::GetTimes( _minTime, _maxTime );
	_grid.Recalc();
	Refresh( FALSE );
	Update();
}

FxReal FxCurveWidget::GetMinimumTime()
{
	FxReal retval = 0.0f;
	if( _pAnim )
	{
		retval = _pAnim->GetStartTime() - 0.1f;
	}
	return retval;
}

FxReal FxCurveWidget::GetMaximumTime()
{
	FxReal retval = 0.0f;
	if( _pAnim )
	{
		retval = _pAnim->GetEndTime() + 0.1f;
	}
	return retval;
}

FxInt32 FxCurveWidget::GetPaintPriority()
{
	return 2;
}

FxBool FxCurveWidget::GetCurveVisibility( const FxName& curve )
{
	FxBool retval = FxFalse;
	if( _pAnimController )
	{
		retval = _pAnimController->IsCurveVisible(curve);
	}
	return retval;
}

void FxCurveWidget::SetCurveVisibility( const FxName& curve, FxBool isVisible )
{
	if( _pAnimController )
	{
		_pAnimController->SetCurveVisible(curve, isVisible);
	}
}

void FxCurveWidget::ClearCurveVisibility()
{
	if( _pAnimController )
	{
		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			_pAnimController->SetCurveVisible(i, FxFalse);
		}
	}
}

wxColour FxCurveWidget::GetCurveColour( const FxName& curve )
{
	if( _pAnimController )
	{
		return _pAnimController->GetCurveColour(curve);
	}
	return *wxBLACK;
}

void FxCurveWidget::SetCurveColour( const FxName& curve, const wxColour& colour )
{
	if( _pAnimController )
	{
		_pAnimController->SetCurveColour(curve, colour);
	}
}

void FxCurveWidget::GetGraphRange( FxReal& minValue, FxReal& maxValue )
{
	minValue = _minValue;
	maxValue = _maxValue;
}

void FxCurveWidget::OnAppStartup( FxWidget* FxUnused(sender) )
{
}

void FxCurveWidget::OnAppShutdown( FxWidget* FxUnused(sender) )
{
	FxTimeManager::UnregisterSubscriber(this);
}

void FxCurveWidget::
OnAnimChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim )
{
	_pAnim = anim;
	// Calculate a good min/max value.
	if( _pAnim	)
	{
		FxReal minKeyValue = FX_REAL_MAX;
		FxReal maxKeyValue = FX_REAL_MIN;
		for( FxSize i = 0; i < _pAnim->GetNumAnimCurves(); ++i )
		{
			const FxAnimCurve& curve = _pAnim->GetAnimCurve(i);
			for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
			{
				FxReal keyValue = curve.GetKey(j).GetValue();
				minKeyValue = FxMin( keyValue, minKeyValue );
				maxKeyValue = FxMax( keyValue, maxKeyValue );
			}
		}
		if( !FxRealEquality(minKeyValue, FX_REAL_MAX) && 
			!FxRealEquality(maxKeyValue, FX_REAL_MIN) )
		{
			FxReal cushion = (maxKeyValue - minKeyValue) * 0.05f;
			_minValue = minKeyValue - cushion;
			_maxValue = maxKeyValue + cushion;
		}
		else
		{
			_maxValue = 1.1f;
			_minValue = -0.1f;
		}
	}
	else
	{
		_maxValue = 1.1f;
		_minValue = -0.1f;
	}
	SetToolbarState();
	_grid.Recalc();
	Refresh(FALSE);
}

void FxCurveWidget::OnAnimCurveSelChanged( FxWidget* FxUnused(sender), 
						                   const FxArray<FxName>& selAnimCurveNames )
{
	_hasCurveSelection = selAnimCurveNames.Length() > 0;
	if( _pAnimController )
	{
		_pAnimController->RecalcPivotKey();
	}

	SetToolbarState();
	Refresh(FALSE);
	Update();
}

void FxCurveWidget::OnRefresh( FxWidget* FxUnused(sender) )
{
	SetToolbarState();
	UpdateToolbarDisplays(FxTrue);
	wxRect workRect = GetWorkRect();
	Refresh(FALSE, &workRect);
	Update();
}

void FxCurveWidget::OnTimeChanged( FxWidget* FxUnused(sender), FxReal newTime )
{
	_currentTime = newTime;
	_grid.SetCurrTime(_currentTime);
	wxRect workRect = GetWorkRect();
	Refresh(FALSE, &workRect);
	Update();
}

void FxCurveWidget::OnRecalculateGrid( FxWidget* FxUnused(sender) )
{
	_grid.Recalc();
	Refresh(FALSE);
}

void FxCurveWidget::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxRect clientRect(GetWorkRect());
	wxPaintDC front( this );

	wxBrush backgroundBrush( wxColour(192, 192, 192), wxSOLID );

	wxMemoryDC dc;
	wxBitmap backbuffer( front.GetSize().GetWidth(), front.GetSize().GetHeight() );
	dc.SelectObject( backbuffer );
	dc.SetBackground( backgroundBrush  );
	dc.Clear();

	wxMemoryDC bitmapDC;
	bitmapDC.SelectObject( _background );
	dc.Blit( wxPoint( clientRect.GetRight() - _background.GetWidth(), 
		              clientRect.GetBottom() - _background.GetHeight() ), 
		bitmapDC.GetSize(), &bitmapDC, bitmapDC.GetLogicalOrigin() );

	if( _pAnimController )
	{
		if( _pAnimController->OK() && _hasCurveSelection )
		{
			// Draw the grid
			DrawGrid( &dc );

			// Draw the hint line
			if( _currentTime != FxInvalidValue )
			{
				FxInt32 timeHintPixel = FxTimeManager::TimeToCoord( _currentTime, clientRect );
				dc.SetPen( wxPen(wxColour(100, 100, 100), 1, wxDOT) );
				dc.DrawLine( wxPoint( timeHintPixel, clientRect.GetTop() ), wxPoint( timeHintPixel, clientRect.GetBottom() ) );
			}

			// Draw the curves
			DrawCurves( &dc );

			for( FxSize currCurve = 0; currCurve < _pAnimController->GetNumCurves(); ++currCurve )
			{
				if( _pAnimController->IsCurveVisible(currCurve) && !_pAnimController->IsCurveOwnedByAnalysis(currCurve) )
				{
					DrawKeys( &dc, currCurve );
				}
			}

			// Draw the selection rectangle
			if( _mouseOp == MOUSE_DRAGSELECTION )
			{
				FxInt32 function = dc.GetLogicalFunction();
				dc.SetLogicalFunction( wxXOR );
				dc.SetPen( wxPen( wxColour( *wxWHITE ), 1, wxDOT ) );
				dc.SetBrush( *wxTRANSPARENT_BRUSH );
				wxRect selection( wxPoint( FxTimeManager::TimeToCoord( _selection.GetLeft(), clientRect ),
					ValueToCoord( _selection.GetTop(), clientRect ) ),
					wxPoint( FxTimeManager::TimeToCoord( _selection.GetRight(), clientRect ),
					ValueToCoord( _selection.GetBottom(), clientRect ) ) );
				dc.DrawRectangle( selection );
				dc.SetLogicalFunction( function );
			}

			// Clean up
			dc.SetBrush( wxNullBrush );
			dc.SetPen( wxNullPen );
		}
	}

	if( _hasFocus )
	{
		dc.SetBrush( *wxTRANSPARENT_BRUSH );
		dc.SetPen( wxPen(FxStudioOptions::GetFocusedWidgetColour(), 1, wxSOLID) );
		dc.DrawRectangle( clientRect );
	}

	wxColour colourFace = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	wxColour colourShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	wxRect toolbarRect = GetToolbarRect();

	// Draw the toolbar background.
	dc.SetBrush(wxBrush(colourFace));
	dc.SetPen(wxPen(colourFace));
	dc.DrawRectangle(toolbarRect);

	// Shade the edges.
	dc.SetPen(wxPen(colourShadow));
	dc.DrawLine(toolbarRect.GetLeft(), toolbarRect.GetBottom(),
		toolbarRect.GetRight(), toolbarRect.GetBottom());

	dc.SetPen(wxNullPen);
	dc.SetBrush(wxNullBrush);

	// Swap to the front buffer
	front.Blit( front.GetLogicalOrigin(), front.GetSize(), &dc, dc.GetLogicalOrigin(), wxCOPY );
}

void FxCurveWidget::OnMouseWheel( wxMouseEvent& event )
{
	if( _pAnimController )
	{
		if( _pAnimController->OK() && _hasCurveSelection )
		{
			eZoomDir zoomDir = ZOOM_OUT;
			if( event.GetWheelRotation() < 0 )
			{
				zoomDir = ZOOM_IN;
			}

			if( !event.ControlDown() && !event.ShiftDown() && !event.AltDown() )
			{
				ZoomTime(event.GetPosition(), _mouseWheelZoomFactor, zoomDir);
			}
			else if( event.ShiftDown() )
			{
				ZoomValue(event.GetPosition(), _mouseWheelZoomFactor, zoomDir);
				Refresh(FALSE);
			}
		}
	}
}

void FxCurveWidget::OnLeftButtonDown( wxMouseEvent& event )
{
	SetFocus();
	// Ignore clicks in the toolbar.
	if( GetToolbarRect().Contains(event.GetPosition()) )
	{
		return;
	}

	_oneDeeTransformDirection = DIR_UNCERTAIN;

	if( _pAnimController )
	{
		if( _pAnimController->OK() && _hasCurveSelection )
		{
			if( !HasCapture() )
			{
				CaptureMouse();
			}
			SetCursor( *wxSTANDARD_CURSOR );

			if( _mouseOp == MOUSE_ADDKEY )
			{
				FxReal mouseTime = FxTimeManager::CoordToTime(event.GetX(), GetWorkRect());
				FxReal mouseValue = CoordToValue(event.GetY(), GetWorkRect());
				wxString command = wxString::Format(wxT("key -insert -default -time %f -value %f"),
					mouseTime, mouseValue);
				if( FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc))) == CE_Success )
				{
					CachePivotKeyStartPos();
					_mouseOp = MOUSE_MOVESELECTION;
					_lastPos = event.GetPosition();
					_startMousePos = wxPoint2DDouble(mouseTime, mouseValue);
					SetToolbarState();
					Refresh( FALSE );
					return;
				}
			}

			if( event.LeftDown() && !event.ShiftDown() && !event.ControlDown() )
			{
				wxRect clientRect = GetWorkRect();
				FxInt32 tempCurve = _editCurve;
				FxInt32 tempKey   = _editKey;

				FxSize curveIndex = FxInvalidIndex;
				FxSize keyIndex = FxInvalidIndex;

				FxReal clickTime = FxTimeManager::CoordToTime( event.GetX(), clientRect );
				FxReal clickValue = CoordToValue( event.GetY(), clientRect );
				FxReal timeEpsilon = FxTimeManager::CoordToTime(3, clientRect) - FxTimeManager::CoordToTime(0, clientRect);
				FxReal valueEpsilon = CoordToValue(0, clientRect) - CoordToValue(3, clientRect);

				FxInt32 which;
				if( _pAnimController->HitTestTanHandles( event.GetX(), event.GetY(), 3, 3, which ) )
				{
					FxSize pivotCurve, pivotKey;
					_pAnimController->GetPivotKey(pivotCurve, pivotKey);
					FxReal trash;
					_pAnimController->GetKeyInfo(pivotCurve, pivotKey, trash, trash, _cachedSlopeIn, _cachedSlopeOut);
					_editTangent = static_cast<eWhichTangent>(which);
					_mouseOp = MOUSE_EDITTANGENTS;
					Refresh(FALSE);
				}
				else if( _pAnimController->HitTest( FxAnimController::HitTestSelected,
					clickTime, clickValue, timeEpsilon, valueEpsilon, &curveIndex, &keyIndex ) )
				{
					if( curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex && !_pAnimController->IsKeyPivot(curveIndex,keyIndex))
					{
						wxString command = wxString::Format(wxT("key -pivot -set -curveIndex %u -keyIndex %u"), curveIndex, keyIndex);
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
					CachePivotKeyStartPos();
					_mouseOp = MOUSE_MOVESELECTION;
					_startMousePos = wxPoint2DDouble(FxTimeManager::CoordToTime(event.GetX(), GetWorkRect()),
						CoordToValue(event.GetY(), GetWorkRect()));
					Refresh(FALSE);
				}
				else if( _pAnimController->HitTest( FxAnimController::HitTestAll,
					clickTime, clickValue, timeEpsilon, valueEpsilon, &curveIndex, &keyIndex ) )
				{
					if( curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
					{
						wxString command = wxString::Format(wxT("key -select -curveIndex %u -keyIndex %u"),
							curveIndex, keyIndex );
						if( FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc))) == CE_Success )
						{
							_mouseOp = MOUSE_MOVESELECTION;
							CachePivotKeyStartPos();
							_startMousePos = wxPoint2DDouble(FxTimeManager::CoordToTime(event.GetX(), GetWorkRect()),
								CoordToValue(event.GetY(), GetWorkRect()));

							Refresh(FALSE);
						}
					}
				}
				else
				{
					wxPoint pos = event.GetPosition();
					_mouseOp = MOUSE_DRAGSELECTION;
					_selection.SetTop( CoordToValue( pos.y, GetWorkRect() ) );
					_selection.SetBottom( _selection.GetTop() );
					_selection.SetLeft( FxTimeManager::CoordToTime( pos.x, GetWorkRect() ) );
					_selection.SetRight( _selection.GetLeft() );
					_editCurve = tempCurve;
					_editKey   = tempKey;
				}
			}
			else if( event.LeftDown() && event.ShiftDown() && !event.ControlDown() )
			{
				CachePivotKeyStartPos();
				_mouseOp = MOUSE_MOVESELECTION;
				_startMousePos = wxPoint2DDouble(FxTimeManager::CoordToTime(event.GetX(), GetWorkRect()),
					CoordToValue(event.GetY(), GetWorkRect()));
			}
			else if( event.LeftDown() && event.ControlDown() && !event.ShiftDown() )
			{
				wxPoint pos = event.GetPosition();
				_mouseOp = MOUSE_DRAGSELECTION;
				_selection.SetTop( CoordToValue( pos.y, GetWorkRect() ) );
				_selection.SetBottom( _selection.GetTop() );
				_selection.SetLeft( FxTimeManager::CoordToTime( pos.x, GetWorkRect() ) );
				_selection.SetRight( _selection.GetLeft() );
			}
			else if( event.LeftDown() && event.ControlDown() && event.ShiftDown() )
			{
				CachePivotKeyStartPos();
				_mouseOp = MOUSE_ONEDMOVESELECTION;
				_startMousePos = wxPoint2DDouble(FxTimeManager::CoordToTime(event.GetX(), GetWorkRect()),
					CoordToValue(event.GetY(), GetWorkRect()));
			}
			_lastPos = event.GetPosition();
		}
	}
}

void FxCurveWidget::OnLeftButtonUp( wxMouseEvent& event )
{
	// Ignore clicks in the toolbar.
	if( GetToolbarRect().Contains(event.GetPosition()) && _mouseOp == MOUSE_NONE )
	{
		return;
	}

	if( _pAnimController )
	{
		if( _pAnimController->OK() && _hasCurveSelection )
		{
			if( HasCapture() )
			{
				ReleaseMouse();
			}
			if( _mouseOp == MOUSE_DRAGSELECTION )
			{
				if( _selection.GetLeftTop() != _selection.GetRightBottom() )
				{
					FxReal minValue = FxMin( _selection.GetBottom(), _selection.GetTop() );
					FxReal maxValue = FxMax( _selection.GetBottom(), _selection.GetTop() );
					FxReal minTime  = FxMin( _selection.GetLeft(), _selection.GetRight() );
					FxReal maxTime  = FxMax( _selection.GetLeft(), _selection.GetRight() );

					if( !event.ControlDown() )
					{
						wxString command = wxString::Format(wxT("key -select -minTime %f -maxTime %f -minValue %f -maxValue %f"),
							minTime, maxTime, minValue, maxValue);
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
					else
					{
						wxString command = wxString::Format(wxT("key -select -toggle -minTime %f -maxTime %f -minValue %f -maxValue %f"),
							minTime, maxTime, minValue, maxValue);
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
					
					if( _pAnimController->GetEditCurve() != FxInvalidIndex &&
						_pAnimController->GetEditKey() != FxInvalidIndex )
					{
						FxReal trash;
						_pAnimController->GetKeyInfo(_pAnimController->GetEditCurve(), _pAnimController->GetEditKey(),
							_keyeditTime, _keyeditValue, trash, trash);
						SetToolbarState();
						TransferDataToWindow();
					}
					else
					{
						SetToolbarState();
					}
				}
				else if( _pAnimController->GetNumSelectedKeys() > 0 )
				{
					FxVM::ExecuteCommand("key -select -clear");
					SetToolbarState();
				}
				_selection.SetLeftTop( wxPoint2DDouble( 0.0f, 0.0f ) );
				_selection.SetRightBottom( wxPoint2DDouble( 0.0f, 0.0f ) );
			}
			else if( _mouseOp == MOUSE_MOVESELECTION ||
					 _mouseOp == MOUSE_ONEDMOVESELECTION )
			{
				// Get the pivot key's time/value and calculate the delta from its starting position.
				FxSize curveIndex, keyIndex;
				wxPoint2DDouble endPos;
				_pAnimController->GetPivotKey(curveIndex, keyIndex);
				if( curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
				{
					FxReal time, value, trash;
					_pAnimController->GetKeyInfo(curveIndex, keyIndex, time, value, trash, trash);
					endPos.m_x = time;
					endPos.m_y = value;
				}

				wxPoint2DDouble delta = endPos - _startPos;

				FxReal deltaTime  = delta.m_x;
				FxReal deltaValue = delta.m_y;
				wxString command(wxT("key -transform -fromgui"));
				if( !FxRealEquality(deltaTime, 0.0f) )
				{
					command += wxString::Format(wxT(" -time %f"), deltaTime);
				}
				if( !FxRealEquality(deltaValue, 0.0f) )
				{
					command += wxString::Format(wxT(" -value %f"), deltaValue);
				}
				if( command != wxT("key -transform -fromgui") )
				{
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
			else if( _mouseOp == MOUSE_EDITTANGENTS )
			{
				FxSize pivotCurve = FxInvalidIndex;
				FxSize pivotKey = FxInvalidIndex;
				_pAnimController->GetPivotKey( pivotCurve, pivotKey );
				if( pivotCurve != FxInvalidIndex && pivotKey != FxInvalidIndex )
				{
					FxReal keyTime, keyValue, slopeIn, slopeOut;
					_pAnimController->GetKeyInfo( pivotCurve, pivotKey, 
						keyTime, keyValue, slopeIn, slopeOut );
					wxString command = wxString::Format(wxT("key -edit -curveIndex %u -keyIndex %u -slopeIn %f -slopeOut %f -psi %f -pso %f"),
						pivotCurve, pivotKey, slopeIn, slopeOut, _cachedSlopeIn, _cachedSlopeOut );
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}

			_mouseOp = MOUSE_NONE;
			Refresh( FALSE );
		}
	}

	_oneDeeTransformDirection = DIR_UNCERTAIN;
}

void FxCurveWidget::OnMouseMove( wxMouseEvent& event )
{
	if( _pAnimController )
	{
		if( _pAnimController->OK() && _hasCurveSelection )
		{
			wxRect clientRect = GetWorkRect();
			wxPoint thisPos = event.GetPosition();
			FxReal thisTime  = FxTimeManager::CoordToTime( thisPos.x, clientRect );
			FxReal thisValue = CoordToValue( thisPos.y, clientRect );
			if( event.MiddleIsDown() && !event.LeftIsDown() )
			{
				if( _mouseOp == MOUSE_MOVETIME )
				{
					FxReal currentTime = _grid.SnapTime(FxTimeManager::CoordToTime(thisPos.x, clientRect), FxTrue, _pAnimController);
					if( _mediator )
					{
						_mediator->OnTimeChanged(NULL, currentTime);
					}
				}
				else if( _mouseOp == MOUSE_DRAGSELECTION )
				{
					// Adjust the bottom right corner of the selection
					wxPoint pos = event.GetPosition();
					_selection.SetBottom( thisValue );
					_selection.SetRight( thisTime );
					Refresh( FALSE );
					Update();
				}
				else
				{
					FxReal deltaValue = CoordToValue(thisPos.y, clientRect) - CoordToValue(_lastPos.y, clientRect);
					FxReal deltaTime  = FxTimeManager::CoordToTime(thisPos.x, clientRect) - FxTimeManager::CoordToTime(_lastPos.x, clientRect);
					if( _oneDeePan )
					{
						if( _oneDeePanDirection == DIR_UNCERTAIN )
						{
							if( FxAbs(deltaValue) > FxAbs(deltaTime) )
							{
								_oneDeePanDirection = DIR_VALUE;
							}
							else if( FxAbs(deltaTime) > FxAbs(deltaValue) )
							{
								_oneDeePanDirection = DIR_TIME;
							}
							else
							{
								// Not enough information to begin a 1D pan, try again.
								return;
							}
						}

						if( _oneDeePanDirection == DIR_VALUE )
						{
							deltaTime = 0.0f;
						}
						else if( _oneDeePanDirection == DIR_TIME )
						{
							deltaValue = 0.0f;
						}
					}

					if( !FxRealEquality(deltaTime, 0.0f) )
					{
						if( FxTimeManager::RequestTimeChange( _minTime - deltaTime, _maxTime - deltaTime ) )
						{
							if( !FxRealEquality(deltaValue, 0.0f) )
							{
								_minValue -= deltaValue;
								_maxValue -= deltaValue;
								_grid.Recalc();
							}
						}
					}
					else if( !FxRealEquality(deltaValue, 0.0f) )
					{
						_minValue -= deltaValue;
						_maxValue -= deltaValue;
						_grid.Recalc();
						Refresh(FALSE);
					}
				}
			}
			else if( event.LeftIsDown() )
			{
				if( _mouseOp == MOUSE_DRAGSELECTION )
				{
					// Adjust the bottom right corner of the selection
					wxPoint pos = event.GetPosition();
					_selection.SetBottom( thisValue );
					_selection.SetRight( thisTime );
					Refresh( FALSE );
					Update();
				}
				else if ( _mouseOp == MOUSE_MOVESELECTION || _mouseOp == MOUSE_ONEDMOVESELECTION )
				{
					// Get the mouse's time/value
					wxPoint2DDouble currentMousePos = wxPoint2DDouble(FxTimeManager::CoordToTime(event.GetX(), GetWorkRect()),
						CoordToValue(event.GetY(), GetWorkRect()));
					wxPoint2DDouble mouseDelta = currentMousePos - _startMousePos;

					FxReal desiredTime = mouseDelta.m_x;
					FxReal desiredValue = mouseDelta.m_y;
					if( _mouseOp == MOUSE_ONEDMOVESELECTION )
					{
						if( _oneDeeTransformDirection == DIR_UNCERTAIN )
						{
							if( FxAbs(desiredTime) > FxAbs(desiredValue) )
							{
								_oneDeeTransformDirection = DIR_TIME;
							}
							else
							{
								_oneDeeTransformDirection = DIR_VALUE;
							}
						}

						if( _oneDeeTransformDirection == DIR_VALUE )
						{
							desiredTime = 0.f;
						}
						else if( _oneDeeTransformDirection == DIR_TIME )
						{
							desiredValue = 0.f;
						}
					}
					if( _buttonSnapTime->GetToggleValue() )
					{
						desiredTime  = _grid.SnapTime( _startPos.m_x + desiredTime );
					}
					else
					{
						desiredTime =  _startPos.m_x + desiredTime;
					}
					if( _buttonSnapValue->GetToggleValue() )
					{
						desiredValue = _grid.SnapValue( _startPos.m_y + desiredValue );
					}
					else
					{
						desiredValue = _startPos.m_y + desiredValue;
					}
					FxReal currentTime = 0.0f;
					FxReal currentValue = 0.0f;
					FxSize pivotCurve = FxInvalidIndex;
					FxSize pivotKey = FxInvalidIndex;
					_pAnimController->GetPivotKey( pivotCurve, pivotKey );
					if( pivotCurve != FxInvalidIndex &&
						pivotKey != FxInvalidIndex )
					{
						FxReal dummyRef;
						_pAnimController->GetKeyInfo( pivotCurve, pivotKey,
							currentTime, currentValue, dummyRef, dummyRef );
					}

					FxReal deltaTime  = desiredTime - currentTime;
					FxReal deltaValue = desiredValue - currentValue;

					// Apply the snapped transform to the entire selection
					_pAnimController->TransformSelection( deltaTime, deltaValue );

					Refresh(FALSE);
					Update();
					//@todo Is this how we really want this to work?
					if( _mediator )
					{
						_mediator->OnRefresh(this);
					}
				}
				else if( _mouseOp == MOUSE_EDITTANGENTS )
				{
					FxSize pivotCurve = FxInvalidIndex;
					FxSize pivotKey = FxInvalidIndex;
					_pAnimController->GetPivotKey( pivotCurve, pivotKey );
					if( pivotCurve != FxInvalidIndex && pivotKey != FxInvalidIndex )
					{
						wxRect clientRect = GetWorkRect();
						wxPoint pos = event.GetPosition();
						FxReal keyTime, keyValue, slopeIn, slopeOut;
						_pAnimController->GetKeyInfo( pivotCurve, pivotKey, 
							keyTime, keyValue, slopeIn, slopeOut );

						FxReal slope = (thisValue - keyValue) / (thisTime - keyTime);

						if( _pAnimController->IsTangentLocked( pivotCurve, pivotKey ) )
						{
							_pAnimController->ModifyPivotKey( keyTime, keyValue, slope, slope, FxFalse );
						}
						else if( _editTangent == TANGENT_INCOMING )
						{
							_pAnimController->ModifyPivotKey( keyTime, keyValue, slope, slopeOut, FxFalse );
						}
						else if( _editTangent == TANGENT_OUTGOING )
						{
							_pAnimController->ModifyPivotKey( keyTime, keyValue, slopeIn, slope, FxFalse );
						}
					}
					Refresh(FALSE);
					Update();
					//@todo Is this how we really want this to work?
					if( _mediator )
					{
						_mediator->OnRefresh(this);
					}
				}
			}

			_lastPos = thisPos;
			_lastTime = thisTime;
			_lastValue = thisValue;

			UpdateToolbarDisplays();
		}
	}
}

void FxCurveWidget::OnRightButtonDown( wxMouseEvent& FxUnused(event) )
{
	SetFocus();
}

void FxCurveWidget::OnRightButtonUp( wxMouseEvent& event )
{
	// Ignore clicks in the toolbar.
	if( GetToolbarRect().Contains(event.GetPosition()) )
	{
		return;
	}

	if( _pAnimController )
	{
		if( _pAnimController->OK() && _hasCurveSelection )
		{
			wxRect clientRect = GetWorkRect();

			FxReal timeEpsilon = FxTimeManager::CoordToTime( event.GetX(), clientRect ) -
				FxTimeManager::CoordToTime( event.GetX() - 3, clientRect );
			FxReal valueEpsilon = CoordToValue( event.GetY(), clientRect ) - 
				CoordToValue( event.GetY() + 3, clientRect );

			wxMenu menu;
			FxSize curve, key;
			if( _pAnimController->HitTest( FxAnimController::HitTestSelected, 
				FxTimeManager::CoordToTime( event.GetX(), clientRect ),
				CoordToValue( event.GetY(), clientRect ),
				timeEpsilon, valueEpsilon, &curve, &key ) )
			{
				if( curve != FxInvalidIndex && key != FxInvalidIndex && !_pAnimController->IsKeyPivot(curve, key) )
				{
					wxString command = wxString::Format(wxT("key -pivot -set -curveIndex %u -keyIndex %u"), curve, key);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}

				if( _pAnimController->IsTangentLocked( curve, key ) )
				{
					menu.Append( MenuID_CurveWidgetUnlockTangent, _("&Unlock Tangents"), _("Allows the tangent handles to be moved independantly of one another.") );
				}
				else
				{
					menu.Append( MenuID_CurveWidgetLockTangent, _("&Lock Tangents"), _("Forces the incoming and outgoing tangents to be colinear.") );
				}
				menu.AppendSeparator();
			}

			FxBool visibleKeySelected = FxFalse;
			for( FxSize i = 0; i < _pAnimController->GetNumCurves() && !visibleKeySelected; ++i )
			{
				if( _pAnimController->IsCurveVisible( i ) )
				{
					for( FxSize j = 0; j < _pAnimController->GetNumKeys(i); ++j)
					{
						if( _pAnimController->IsKeySelected(i, j) )
						{
							visibleKeySelected = FxTrue;
							break;
						}
					}
				}
			}
			menu.Append( MenuID_CurveWidgetCut, _("Cu&t\tCtrl+X"), _("Cut the selected keys") );
			menu.Enable( MenuID_CurveWidgetCut, visibleKeySelected );
			menu.Append( MenuID_CurveWidgetCopy, _("&Copy\tCtrl+C"), _("Copy the selected keys") );
			menu.Enable( MenuID_CurveWidgetCopy, visibleKeySelected );
			menu.Append( MenuID_CurveWidgetPaste, _("&Paste\tCtrl+V"), _("Paste the copied/cut keys") );
			menu.Enable( MenuID_CurveWidgetPaste, _copyBuffer.Length()!=0 );
			PopupMenu( &menu, event.GetPosition() );
			return;
		}
	}
}

void FxCurveWidget::OnMiddleButtonDown( wxMouseEvent& event )
{
	SetFocus();
	if( !HasCapture() )
	{
		CaptureMouse();
	}
	if( _pAnimController )
	{
		if( _pAnimController->OK() && _hasCurveSelection && !event.LeftIsDown() ) 
		{
			if( event.ShiftDown() && event.ControlDown() )
			{
				wxPoint pos = event.GetPosition();
				_mouseOp = MOUSE_DRAGSELECTION;
				_selection.SetTop( CoordToValue( pos.y, GetWorkRect() ) );
				_selection.SetBottom( _selection.GetTop() );
				_selection.SetLeft( FxTimeManager::CoordToTime( pos.x, GetWorkRect() ) );
				_selection.SetRight( _selection.GetLeft() );
			}
			else if( event.ShiftDown() )
			{
				_oneDeePan = FxTrue;
				_oneDeePanDirection = DIR_UNCERTAIN;
			}
			else if( event.ControlDown() )
			{
				_mouseOp = MOUSE_MOVETIME;
				_cachedCurrentTime = _currentTime;
				if( _mediator )
				{
					_mediator->OnStopAnimation(NULL);
				}
			}
			else
			{
				_oneDeePan = FxFalse;
			}
		}
	}
}

void FxCurveWidget::OnMiddleButtonUp( wxMouseEvent& event )
{
	if( HasCapture() )
	{
		ReleaseMouse();
	}
	if( _pAnimController )
	{
		if( _mouseOp == MOUSE_MOVETIME )
		{
			FxReal currentTime = _grid.SnapTime(FxTimeManager::CoordToTime(event.GetX(), GetClientRect()), FxTrue, _pAnimController);
			wxString command = wxString::Format(wxT("currentTime -new %f -prev %f"), currentTime, _cachedCurrentTime);
			FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			_mouseOp = MOUSE_NONE;

			// Due to the floating point precision issues of going back and forth
			// from floats to strings in the command system, and the fact that we
			// need a *very* precise value to come across the wire in the slider
			// widget for key icon highlighting, re-dispatch the OnTimeChanged
			// message to get an unadulterated version of the time through to the
			// slider widget.
			if( _mediator )
			{
				_mediator->OnTimeChanged(this, currentTime);
			}
		}
		else if( _mouseOp == MOUSE_DRAGSELECTION && event.ControlDown() && event.ShiftDown() )
		{
			FxReal minValue = FxMin( _selection.GetBottom(), _selection.GetTop() );
			FxReal maxValue = FxMax( _selection.GetBottom(), _selection.GetTop() );
			FxReal minTime  = FxMin( _selection.GetLeft(), _selection.GetRight() );
			FxReal maxTime  = FxMax( _selection.GetLeft(), _selection.GetRight() );

			_minValue = minValue;
			_maxValue = maxValue;
			if( _maxValue - _minValue < 0.0001 )
			{
				_maxValue = _minValue + 0.0001;
			}
			FxTimeManager::RequestTimeChange( minTime, maxTime );
			_mouseOp = MOUSE_NONE;
		}

		if( _pAnimController->OK() && _hasCurveSelection )
		{
			_oneDeePan = FxFalse;
			_oneDeePanDirection = DIR_UNCERTAIN;
		}
	}
}

void FxCurveWidget::OnKeyDown( wxKeyEvent& FxUnused(event) )
{
}

// Called when the window is resized.
void FxCurveWidget::OnResize( wxSizeEvent& FxUnused(event) )
{
	_grid.Recalc();
	Layout();
}

void FxCurveWidget::OnResetView( wxCommandEvent& FxUnused(event) )
{
	FxTimeManager::RequestTimeChange( FxTimeManager::GetMinimumTime(), 
									   FxTimeManager::GetMaximumTime() );
	CalcVerticalSpan();
}

void FxCurveWidget::OnSelectAll( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController && _hasCurveSelection )
	{
		FxVM::ExecuteCommand("key -select");
	}
}

void FxCurveWidget::OnLockTangent( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController && _hasCurveSelection )
	{
		// Tangent locking now affects all selected keys.
		_pAnimController->SetSelectionTangentLocked(FxTrue);
		Refresh(FALSE);
	}
}

void FxCurveWidget::OnUnlockTangent( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController && _hasCurveSelection )
	{
		// Tangent locking now affects all selected keys.
		_pAnimController->SetSelectionTangentLocked(FxFalse);
		Refresh(FALSE);
	}
}

void FxCurveWidget::OnDeleteSelection( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController && _hasCurveSelection && _pAnimController->GetNumSelectedKeys() > 0 )
	{
		FxVM::ExecuteCommand("key -removeSelection");
		SetToolbarState();
	}
}

void FxCurveWidget::OnAddKey( wxCommandEvent& FxUnused(event) )
{
	if( _hasCurveSelection )
	{
		_mouseOp = MOUSE_ADDKEY;
		SetCursor( *wxCROSS_CURSOR );
	}
}

void FxCurveWidget::OnResetSlopes( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController && _hasCurveSelection && _pAnimController->GetNumSelectedKeys() > 0 )
	{
		if( wxYES == FxMessageBox(_("This will reset the slopes of all the selected keys to zero and is not undoable.  Continue?"),
			_("Confirm Opertaion"), wxYES_NO|wxICON_QUESTION, this) )
		{
			_pAnimController->TransformSelection(0.0f, 0.0f, 0.0f );
			Refresh( FALSE );
		}
	}
}

void FxCurveWidget::OnCopy( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController && _hasCurveSelection )
	{
		_copyBuffer.Clear();
		_copyMinTime = FX_REAL_MAX;
		FxSize count = 0;
		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			if( _pAnimController->IsCurveVisible(i) )
			{
				_copyBuffer.PushBack(FxArray<FxAnimKey>());
				for( FxSize j = 0; j < _pAnimController->GetNumKeys(i); ++j )
				{
					if( _pAnimController->IsKeySelected(i,j) )
					{
						FxReal time, value, slopeIn, slopeOut;
						_pAnimController->GetKeyInfo( i, j, time, value, slopeIn, slopeOut );
						_copyBuffer[count].PushBack( FxAnimKey(time, value, slopeIn, slopeOut) );
						_copyMinTime = FxMin(_copyMinTime, time);
					}
				}
				count++;
			}
		}
	}
	SetToolbarState();
}

void FxCurveWidget::OnCut( wxCommandEvent& event )
{
	if( _pAnimController && _hasCurveSelection )
	{
		OnCopy(event);
		FxVM::ExecuteCommand("key -removeSelection");
	}
}

void FxCurveWidget::OnPaste( wxCommandEvent& FxUnused(event) )
{
	// Deselect everything before we paste.
	FxVM::ExecuteCommand("key -select -clear");
	if( _pAnimController && _hasCurveSelection && _copyBuffer.Length() )
	{
		FxVM::ExecuteCommand("batch");

		if( _copyBuffer.Length() == 1 )
		{
			for( FxSize i = 0; i < _copyBuffer[0].Length(); ++i )
			{
				FxAnimKey& key = _copyBuffer[0][i];
				for( FxSize curve = 0; curve < _pAnimController->GetNumCurves(); ++curve )
				{
					if( _pAnimController->IsCurveVisible(curve) )
					{
						wxString command = wxString::Format(wxT("key -insert -curveIndex %u -time %f -value %f -slopeIn %f -slopeOut %f -noClearSelection"),
							curve, key.GetTime() - _copyMinTime + _currentTime, key.GetValue(), key.GetSlopeIn(), key.GetSlopeOut());
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
				}
			}				
		}
		else
		{
			FxSize count = 0;
			for( FxSize curve = 0; curve < _pAnimController->GetNumCurves(); ++curve )
			{
				if( _pAnimController->IsCurveVisible(curve) )
				{
					if( _copyBuffer.Length() > count )
					{
						for( FxSize i = 0; i < _copyBuffer[count].Length(); ++i )
						{
							FxAnimKey& key = _copyBuffer[count][i];
							wxString command = wxString::Format(wxT("key -insert -curveIndex %u -time %f -value %f -slopeIn %f -slopeOut %f -noClearSelection"),
								curve, key.GetTime() - _copyMinTime + _currentTime, key.GetValue(), key.GetSlopeIn(), key.GetSlopeOut());
							FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
						}
						count++;
					}
				}
			}
		}

		FxVM::ExecuteCommand("execBatch -editedcurves");
		_pAnimController->RecalcPivotKey();
		Refresh(FALSE);
	}
}

void FxCurveWidget::OnGetFocus( wxFocusEvent& FxUnused(event) )
{
	_mouseOp = MOUSE_NONE;
	_hasFocus = FxTrue;
}

void FxCurveWidget::OnLoseFocus( wxFocusEvent& FxUnused(event) )
{
	_hasFocus = FxFalse;
	Refresh(FALSE);
}

void FxCurveWidget::OnFitTime( wxCommandEvent& FxUnused(event) )
{
	FxReal minTime = 0.0f;
	FxReal maxTime = 1.0f;
	if( _pAnimController && _pAnimController->OK() && _hasCurveSelection )
	{
		// Determine the min/max time
		minTime = FX_REAL_MAX;
		maxTime = FX_REAL_MIN;
		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			if( _pAnimController->IsCurveVisible(i) )
			{
				FxReal curveMin, curveMax;
				_pAnim->GetAnimCurve(i).FindTimeExtents(curveMin, curveMax);
				minTime = FxMin(curveMin, minTime);
				maxTime = FxMax(curveMax, maxTime);
			}
		}

		maxTime = maxTime + 0.1f * (maxTime - minTime);
		minTime = minTime - 0.1f * (maxTime - minTime);
		_grid.Recalc();
	}

	// Clamp the result to the valid time range of the time manager.
	minTime = FxMax(FxTimeManager::GetMinimumTime(), minTime);
	maxTime = FxMin(FxTimeManager::GetMaximumTime(), maxTime);

	FxTimeManager::RequestTimeChange( minTime, maxTime );
}

void FxCurveWidget::OnFitValue( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController && _pAnimController->OK() && _hasCurveSelection )
	{
		// Determine the min/max value
		FxReal minValue = FX_REAL_MAX;
		FxReal maxValue = FX_REAL_MIN;

		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			if( _pAnimController->IsCurveVisible(i) )
			{
				FxReal curveMin, curveMax;
				_pAnim->GetAnimCurve(i).FindValueExtents(curveMin, curveMax);
				minValue = FxMin(curveMin, minValue);
				maxValue = FxMax(curveMax, maxValue);
			}
		}

		if( FxRealEquality(minValue, maxValue) )
		{
			maxValue = minValue + 1.0f;
		}

		_maxValue = maxValue + 0.1f * (maxValue - minValue);
		_minValue = minValue - 0.1f * (maxValue - minValue);
		_grid.Recalc();
	}
	else
	{
		_maxValue = 1.0f;
		_minValue = 0.0f;
	}
	Refresh( FALSE );
}

// Shows all active curves.
void FxCurveWidget::OnViewAllActiveCurves( wxCommandEvent& FxUnused(event) )
{
	FxArray<FxName> activeCurveNames;
	if( _pAnimController )
	{
		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			if( !FxRealEquality(_pAnimController->GetCurve(i)->EvaluateAt(_currentTime), 0.0f) )
			{
				activeCurveNames.PushBack(_pAnimController->GetCurve(i)->GetName());
			}
		}
	}
	if( _mediator )
	{
		_mediator->OnAnimCurveSelChanged(NULL, activeCurveNames);
	}
}

void FxCurveWidget::OnNextKey( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController )
	{
		FxReal nextKeyTime = FX_REAL_MAX;

		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			if( _pAnimController->IsCurveVisible(i) )
			{
				for( FxSize j = 0; j < _pAnimController->GetNumKeys(i); ++j )
				{
					FxReal keyTime = _pAnimController->GetKeyTime(i, j);
					if( keyTime > _currentTime && keyTime < nextKeyTime )
					{
						nextKeyTime = keyTime;
					}
				}
			}
		}

		if( nextKeyTime != FX_REAL_MAX && _mediator )
		{
			_mediator->OnTimeChanged(NULL, nextKeyTime);
		}
	}
}

void FxCurveWidget::OnPrevKey( wxCommandEvent& FxUnused(event) )
{
	if( _pAnimController )
	{
		FxReal prevKeyTime = static_cast<FxReal>(FX_INT32_MIN);

		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			if( _pAnimController->IsCurveVisible(i) )
			{
				for( FxSize j = 0; j < _pAnimController->GetNumKeys(i); ++j )
				{
					FxReal keyTime = _pAnimController->GetKeyTime(i, j);
					if( keyTime < _currentTime && keyTime > prevKeyTime )
					{
						prevKeyTime = keyTime;
					}
				}
			}
		}

		if( prevKeyTime != static_cast<FxReal>(FX_INT32_MIN) && _mediator )
		{
			_mediator->OnTimeChanged(NULL, prevKeyTime);
		}
	}
}

// Updates the keys.
void FxCurveWidget::OnUpdateKeys( wxCommandEvent& event )
{
	if( _pAnimController )
	{
		TransferDataFromWindow();
		FxSize curveIndex, keyIndex;
		_pAnimController->GetPivotKey(curveIndex, keyIndex);
		if( curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
		{
			if( _pAnimController->GetNumSelectedKeys() == 1)
			{
				wxString commandFmt(wxT("key -edit -curveIndex %u -keyIndex %u -time %f -value %f -slopeIn %f -slopeOut %f"));
				wxString command;
				if( _pAnimController->IsTangentLocked( _pAnimController->GetEditCurve(), _pAnimController->GetEditKey() ) )
				{
					FxReal newDeriv = _keyeditIncDeriv;
					if( event.GetId() == ControlID_CurveWidgetToolbarOutDerivDisplay )
					{
						newDeriv = _keyeditOutDeriv;
					}
					command = wxString::Format(commandFmt, curveIndex, keyIndex, _keyeditTime, _keyeditValue, newDeriv, newDeriv );
				}
				else
				{
					command = wxString::Format(commandFmt, curveIndex, keyIndex, _keyeditTime, _keyeditValue, _keyeditIncDeriv, _keyeditOutDeriv );
				}
				FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			}
			else
			{
				if( event.GetId() == ControlID_CurveWidgetToolbarOutDerivDisplay || event.GetId() == ControlID_CurveWidgetToolbarIncDerivDisplay )
				{
					FxReal newDeriv = _keyeditIncDeriv;
					if( event.GetId() == ControlID_CurveWidgetToolbarOutDerivDisplay )
					{
						newDeriv = _keyeditOutDeriv;
					}
					wxString command = wxString::Format(wxT("key -transform -slopeIn %f"), newDeriv);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
				else
				{
					wxString command(wxT("key -transform"));
					FxReal time, value, trash;
					_pAnimController->GetKeyInfo(curveIndex, keyIndex, time, value, trash, trash);
					FxReal timeDelta = _keyeditTime - time;
					FxReal valueDelta = _keyeditValue - value;
					if( !FxRealEquality(timeDelta, 0.0f) )
					{
						command += wxString::Format(wxT(" -time %f"), timeDelta);
					}
					if( !FxRealEquality(valueDelta, 0.0f) )
					{
						command += wxString::Format(wxT(" -value %f"), valueDelta);
					}
					if( command != wxT("key -transform") )
					{
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
				}
			}
		}
	}
}

void FxCurveWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Animation Tab"));
}

void FxCurveWidget::CalcVerticalSpan( void )
{
	if( _pAnimController && _pAnimController->OK() && _hasCurveSelection )
	{
		// Determine the min/max value
		FxReal minValue = static_cast<FxReal>(FX_INT32_MAX);
		FxReal maxValue = static_cast<FxReal>(FX_INT32_MIN);
		FxReal time, value, dummy;
		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			if( _pAnimController->IsCurveVisible(i) )
			{
				for( FxSize j = 0; j < _pAnimController->GetNumKeys(i); ++j )
				{
					_pAnimController->GetKeyInfo( i, j, time, value, dummy, dummy );
					minValue = FxMin( minValue, value );
					maxValue = FxMax( maxValue, value );
				}
			}
		}
		if( maxValue <= minValue )
		{
			minValue = 0.0f;
			maxValue = 1.0f;
		}
		_maxValue = maxValue + 0.1f * (maxValue - minValue);
		_minValue = minValue - 0.1f * (maxValue - minValue);
		_grid.Recalc();
	}
	else
	{
		_maxValue = 1.0f;
		_minValue = 0.0f;
	}
	Refresh( FALSE );
}

void FxCurveWidget::DrawGrid( wxDC* pDC )
{
	_grid.DrawOn( pDC );
}

void FxCurveWidget::DrawKeys( wxDC* pDC, FxSize curveIndex )
{
	if( _pAnimController )
	{
		wxRect clientRect = GetWorkRect();

		wxColour curveColour = _pAnimController->GetCurveColour( curveIndex );

		wxBrush unselectedKeyBrush( curveColour, wxSOLID );
		wxBrush selectedKeyBrush( *wxRED, wxSOLID );
		wxBrush pivotKeyBrush( wxColour( 255, 255, 0), wxSOLID );
		wxBrush tangentHandleBrush( wxColour( 100, 128, 200 ), wxSOLID );
		wxPen   tangentHandlePen( wxColour( 255, 255, 255 ), 1, wxSOLID );
		wxPen   curvePen( curveColour, 1, wxSOLID );

		// Draw the keys and their 'handles'
		FxSize numKeys = _pAnimController->GetNumKeys( curveIndex );
		for( FxSize i = 0; i < numKeys; ++i )
		{
			pDC->SetPen( *wxBLACK_PEN );
			pDC->SetBrush( unselectedKeyBrush  );
			FxReal keyTime, keyValue, slopeIn, slopeOut;
			FxBool selected = _pAnimController->IsKeySelected( curveIndex, i );

			_pAnimController->GetKeyInfo( curveIndex, i, keyTime, keyValue, slopeIn, slopeOut );

			FxInt32 keyX = FxTimeManager::TimeToCoord( keyTime, clientRect );
			FxInt32 keyY = ValueToCoord( keyValue, clientRect );
			//dkey.lastX = keyX;
			//dkey.lastY = keyY;

			if( clientRect.Contains( keyX, keyY ) )
			{
				// If the key is selected, draw the tangent handles.
				if( selected )
				{
					// Always want tangent lines to be 50 pixels.
					const FxInt32 tangentLength = 50;

					FxVec2 keyPoint( keyX, keyY );

					// Calculate the position in time/value
					FxVec2 outTan(FxInvalidValue, FxInvalidValue);
					if( i < numKeys - 1 )
					{
						outTan = FxVec2( keyTime + 1, keyValue + slopeOut );
						// Transform the position into x/y space
						outTan.x = FxTimeManager::TimeToCoord( outTan.x, clientRect );
						outTan.y = ValueToCoord( outTan.y, clientRect );
						// Translate to the origin
						outTan -= keyPoint;
						// Scale
						outTan.Normalize();
						outTan *= tangentLength;
						// Translate back
						outTan += keyPoint;
					}

					FxVec2 incTan(FxInvalidValue, FxInvalidValue);
					if( i > 0 )
					{
					// Do the same for the incoming tangent handle.
						incTan = FxVec2( keyTime - 1, keyValue - slopeIn );
						incTan.x = FxTimeManager::TimeToCoord( incTan.x, clientRect );
						incTan.y = ValueToCoord( incTan.y, clientRect );
						incTan -= keyPoint;
						incTan.Normalize();
						incTan *= tangentLength;
						incTan += keyPoint;
					}

					_pAnimController->SetTangentHandles( curveIndex, i, 
						wxPoint( incTan.x, incTan.y ), wxPoint( outTan.x, outTan.y ) );

					// Uncomment this code to turn on the circle "tangent guide"
					//pDC->SetBrush( *wxTRANSPARENT_BRUSH );
					//pDC->DrawCircle( keyX, keyY, tangentLength );

					pDC->SetPen( tangentHandlePen );
					if( outTan.x != FxInvalidValue )
					{
						pDC->DrawLine( keyX, keyY, outTan.x, outTan.y );
					}
					if( incTan.x != FxInvalidValue )
					{
						pDC->DrawLine( keyX, keyY, incTan.x, incTan.y );
					}
					pDC->SetPen( wxNullPen );
					pDC->SetBrush( tangentHandleBrush );
					if( outTan.x != FxInvalidValue )
					{
						pDC->DrawCircle( outTan.x, outTan.y, 2 );
					}
					if( incTan.x != FxInvalidValue )
					{
						pDC->DrawCircle( incTan.x, incTan.y, 2 );
					}
					pDC->SetBrush( unselectedKeyBrush );
				}

				// Select the correct brush for the key
				if( _pAnimController->IsKeyPivot( curveIndex, i ) )
				{
					//pDC->SetBrush( pivotKeyBrush );
					pDC->SetPen( *wxRED_PEN );
				}
				else if( selected )
				{
					//pDC->SetBrush( selectedKeyBrush );
					pDC->SetPen( *wxWHITE_PEN );
				}
				else if( _pAnimController->IsKeyTemporary( curveIndex, i ) )
				{
					pDC->SetPen( *wxGREEN_PEN );
				}

//				wxRect keyRect( keyX, keyY, 5, 5 );
//				keyRect.Offset( wxPoint( -2, -2 ) );

				pDC->DrawCircle( keyX, keyY, 3 );
			}
			if( selected )
			{
				pDC->SetBrush( unselectedKeyBrush );
			}
		}

		pDC->SetBrush( wxNullBrush );
		pDC->SetPen( wxNullPen );
	}
}

struct FxCurveProperty
{
	FxSize index;
	FxBool hasVirtualCounterpart;
	FxBool isVirtual;
};

void FxCurveWidget::DrawCurves( wxDC* pDC )
{
	if( _pAnimController )
	{
		wxRect clientRect = GetWorkRect();

		// Cache the animation controller's current time so that it can be
		// reset when drawing is finished.
		FxReal cachedTime = _pAnimController->GetTime();
		
		FxArray<FxCurveProperty> visibleCurves;
		for( FxSize curve = 0; curve < _pAnimController->GetNumCurves(); ++curve )
		{
			if( _pAnimController->IsCurveVisible(curve) )
			{
				FxCurveProperty curveProperty;
				curveProperty.index = curve;
				curveProperty.hasVirtualCounterpart = _pAnimController->IsCurveVirtual(curve);
				curveProperty.isVirtual = FxFalse;
				visibleCurves.PushBack(curveProperty);
				if( _pAnimController->IsCurveVirtual(curve) )
				{
					FxCurveProperty virtualCurveProperty;
					virtualCurveProperty.index = curve;
					virtualCurveProperty.hasVirtualCounterpart = FxFalse;
					virtualCurveProperty.isVirtual = FxTrue;
					visibleCurves.PushBack(virtualCurveProperty);
				}
			}
		}
		FxSize numVisibleCurves = visibleCurves.Length();

		FxInt32 width = clientRect.GetWidth();
		wxPoint** pointArray = new wxPoint*[numVisibleCurves];
		for( FxSize curve = 0; curve < numVisibleCurves; ++curve )
		{
			pointArray[curve] = new wxPoint[width];
		}

		FxReal currentTime = 0.0f;
		for( FxInt32 pixel = 0; pixel < width; ++pixel )
		{
			currentTime = FxTimeManager::CoordToTime(pixel, clientRect);
			_pAnimController->Evaluate(currentTime, FxTrue);
			for( FxSize curve = 0; curve < numVisibleCurves; ++curve )
			{
				pointArray[curve][pixel].x = pixel;
				if( visibleCurves[curve].isVirtual )
				{
					pointArray[curve][pixel].y = ValueToCoord(
						_pAnimController->GetCurveValue(visibleCurves[curve].index, currentTime),
						clientRect );
				}
				else
				{
					pointArray[curve][pixel].y = ValueToCoord(
						_pAnimController->GetValue(visibleCurves[curve].index, FxTrue), 
						clientRect );
				}
			}
		}

		for( FxSize curve = 0; curve < numVisibleCurves; ++curve )
		{
			if( visibleCurves[curve].hasVirtualCounterpart )
			{
				pDC->SetPen(wxPen(_pAnimController->GetCurveColour(visibleCurves[curve].index), 1, wxDOT));
			}
			else
			{
				pDC->SetPen(wxPen(_pAnimController->GetCurveColour(visibleCurves[curve].index), 1, wxSOLID));
			}
			pDC->DrawLines(width, pointArray[curve]);
			delete [] pointArray[curve];
		}
		delete [] pointArray;
		// Reset the animation controller's internal time.
		_pAnimController->SetTime(cachedTime, FxFalse);
	}
}

FxReal FxCurveWidget::CoordToValue( FxInt32 coord, const wxRect& rect )
{
	FxReal pct = static_cast<FxReal>( coord - rect.GetTop() ) / 
				  static_cast<FxReal>( rect.GetHeight() );
	return _maxValue - (pct * (_maxValue - _minValue));
}

FxInt32 FxCurveWidget::ValueToCoord( FxReal value, const wxRect& rect )
{
	FxReal pct = (value - _minValue) / ( _maxValue - _minValue );
	return rect.GetHeight() - static_cast<FxReal>(rect.GetHeight()) * pct + rect.GetTop();
}

//	FxBool HitTestKeys( wxPoint clickPoint );
//	FxBool HitTestTangentHandles( wxPoint clickPoint );

void FxCurveWidget::ZoomTime( const wxPoint& zoomPoint, FxReal zoomFactor, 
						       eZoomDir zoomDir )
{
	if( zoomDir != ZOOM_NONE )
	{
		wxRect clientRect = GetWorkRect();
		FxReal timeSpan = _maxTime - _minTime;

		// Calculate a couple important ratios
		FxReal time  = FxTimeManager::CoordToTime( zoomPoint.x, clientRect);
		FxReal timePct = (time - _minTime) / timeSpan;

		// Zoom in/out around the point by the 'zoom factor'
		FxReal newTimeSpan = ( zoomDir == ZOOM_IN ) ? (timeSpan * zoomFactor) : (timeSpan / zoomFactor);

		// Recalculate the minimum and maximum times
		FxReal tempMinTime = time - ( newTimeSpan * timePct );
		FxReal tempMaxTime = time + ( newTimeSpan * (1 - timePct ) );
		if( FxTimeManager::RequestTimeChange( tempMinTime, tempMaxTime ) )
		{
			_grid.Recalc();
		}
	}
}

void FxCurveWidget::ZoomValue( const wxPoint& zoomPoint, FxReal zoomFactor,
							    eZoomDir zoomDir )
{
	if( zoomDir != ZOOM_NONE )
	{
		wxRect clientRect = GetWorkRect();
		FxReal valueSpan = _maxValue - _minValue;

		// Calculate a couple important ratios
		FxReal value = CoordToValue( zoomPoint.y, clientRect );
		FxReal valuePct = (value - _minValue) / valueSpan;

		// Zoom in/out around the point by the 'zoom factor'
		FxReal newValueSpan= ( zoomDir == ZOOM_IN ) ? (valueSpan * zoomFactor) : (valueSpan / zoomFactor);
		newValueSpan = FxClamp( 0.01f, newValueSpan, 100000.0f );

		// Recalculate the minimum and maximum values
		_minValue = value - ( newValueSpan * valuePct );
		_maxValue = value + ( newValueSpan * (1 - valuePct ) );
		_grid.Recalc();
		Refresh( FALSE );
	}
}

// Returns the toolbar rect
wxRect FxCurveWidget::GetToolbarRect()
{
	wxRect toolbar = GetClientRect();
	if( toolbar.GetHeight() > FxToolbarHeight )
	{
		toolbar.SetHeight(FxToolbarHeight);
	}
	return toolbar;
}

// Returns the work rect
wxRect FxCurveWidget::GetWorkRect()
{
	wxRect work = GetClientRect();
	if( work.GetHeight() > FxToolbarHeight )
	{
		work.SetHeight(work.GetHeight() - FxToolbarHeight);
		work.SetTop(work.GetTop() + FxToolbarHeight);
	}
	return work;
}

// Updates the time/value displays
void FxCurveWidget::UpdateToolbarDisplays(FxBool overrideFocusCheck)
{
	if( _pAnimController && _pAnimController->OK() && _hasCurveSelection && _textTime )
	{
		if( !_textTime->IsEnabled() )
		{
			// If there is no key selected, show the mouse position.
			_keyeditTime = _lastTime;
			_keyeditValue = _lastValue;
			_keyeditIncDeriv = 0.0f;
			_keyeditOutDeriv = 0.0f;
			TransferDataToWindow();
		}
		else
		{
			wxWindow* focused = wxWindow::FindFocus();
			if( overrideFocusCheck || (focused != _textTime && focused != _textValue &&
									   focused != _textIncDeriv && focused != _textOutDeriv) )
			{
				// Otherwise, show the pivot key position.
				FxSize curveIndex, keyIndex;
				_pAnimController->GetPivotKey(curveIndex, keyIndex);
				if( curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
				{
					_pAnimController->GetKeyInfo(curveIndex, keyIndex, _keyeditTime, _keyeditValue, _keyeditIncDeriv, _keyeditOutDeriv);
					TransferDataToWindow();
					if( focused == _textTime || focused == _textValue || focused == _textIncDeriv || focused == _textOutDeriv )
					{
						static_cast<wxTextCtrl*>(focused)->SetSelection(-1, -1);
					}
				}
			}
		}
	}
}

// Enables/disables the toolbar buttons.
void FxCurveWidget::SetToolbarState()
{
	if( _textTime )
	{
		FxBool hasKeySelection = _hasCurveSelection && _pAnimController && _pAnimController->GetNumSelectedKeys() > 0;

		// cut & copy
		for( FxSize i = 0; i < 2; ++i )
		{
			_toolbarButtons[i]->Enable(hasKeySelection);
		}

		// paste
		_toolbarButtons[2]->Enable(_copyBuffer.Length() > 0 && _hasCurveSelection);

		// tangents
		for( FxSize i = 3; i < 5; ++i )
		{
			_toolbarButtons[i]->Enable(hasKeySelection);
		}
		
		// Snaps
		for( FxSize i = 5; i < _toolbarButtons.Length(); ++i )
		{
			_toolbarButtons[i]->Enable(_hasCurveSelection);
		}

		_textTime->Enable(hasKeySelection);
		_textValue->Enable(hasKeySelection);
		_textIncDeriv->Enable(hasKeySelection);
		_textOutDeriv->Enable(hasKeySelection);
		_buttonViewAllActive->Enable(_pAnim != NULL && _pAnimController != NULL);
	}
}

// One-time initialization of the control
void FxCurveWidget::Initialize()
{
	_background = wxBitmap(FxStudioApp::GetBitmapPath(wxT("Curve_Background.bmp")), wxBITMAP_TYPE_BMP );

	if( _textTime )
	{
		wxString cutPath = FxStudioApp::GetIconPath(wxT("cut.ico"));
		wxString cutDisabledPath = FxStudioApp::GetIconPath(wxT("cut_disabled.ico"));
		if( FxFileExists(cutPath) && FxFileExists(cutDisabledPath) )
		{
			wxIcon cutIcon = wxIcon(wxIconLocation(cutPath));
			cutIcon.SetHeight(16);
			cutIcon.SetWidth(16);
			wxIcon cutDisabledIcon = wxIcon(wxIconLocation(cutDisabledPath));
			cutDisabledIcon.SetHeight(16);
			cutDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarCut - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(cutIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarCut - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(cutDisabledIcon);
		}

		wxString copyPath = FxStudioApp::GetIconPath(wxT("copy.ico"));
		wxString copyDisabledPath = FxStudioApp::GetIconPath(wxT("copy_disabled.ico"));
		if( FxFileExists(copyPath) && FxFileExists(copyDisabledPath) )
		{
			wxIcon copyIcon = wxIcon(wxIconLocation(copyPath));
			copyIcon.SetHeight(16);
			copyIcon.SetWidth(16);
			wxIcon copyDisabledIcon = wxIcon(wxIconLocation(copyDisabledPath));
			copyDisabledIcon.SetHeight(16);
			copyDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarCopy - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(copyIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarCopy - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(copyDisabledIcon);
		}

		wxString pastePath = FxStudioApp::GetIconPath(wxT("paste.ico"));
		wxString pasteDisabledPath = FxStudioApp::GetIconPath(wxT("paste_disabled.ico"));
		if( FxFileExists(pastePath) && FxFileExists(pasteDisabledPath) )
		{
			wxIcon pasteIcon = wxIcon(wxIconLocation(pastePath));
			pasteIcon.SetHeight(16);
			pasteIcon.SetWidth(16);
			wxIcon pasteDisabledIcon = wxIcon(wxIconLocation(pasteDisabledPath));
			pasteDisabledIcon.SetHeight(16);
			pasteDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarPaste - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(pasteIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarPaste - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(pasteDisabledIcon);
		}

		wxString locktangentPath = FxStudioApp::GetIconPath(wxT("locktangent.ico"));
		wxString locktangentDisabledPath = FxStudioApp::GetIconPath(wxT("locktangent_disabled.ico"));
		if( FxFileExists(locktangentPath) && FxFileExists(locktangentDisabledPath) )
		{
			wxIcon locktangentIcon = wxIcon(wxIconLocation(locktangentPath));
			locktangentIcon.SetHeight(16);
			locktangentIcon.SetWidth(16);
			wxIcon locktangentDisabledIcon = wxIcon(wxIconLocation(locktangentDisabledPath));
			locktangentDisabledIcon.SetHeight(16);
			locktangentDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarLockTangent - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(locktangentIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarLockTangent - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(locktangentDisabledIcon);
		}

		wxString unlocktangentPath = FxStudioApp::GetIconPath(wxT("unlocktangent.ico"));
		wxString unlocktangentDisabledPath = FxStudioApp::GetIconPath(wxT("unlocktangent_disabled.ico"));
		if( FxFileExists(unlocktangentPath) && FxFileExists(unlocktangentDisabledPath) )
		{
			wxIcon unlocktangentIcon = wxIcon(wxIconLocation(unlocktangentPath));
			unlocktangentIcon.SetHeight(16);
			unlocktangentIcon.SetWidth(16);
			wxIcon unlocktangentDisabledIcon = wxIcon(wxIconLocation(unlocktangentDisabledPath));
			unlocktangentDisabledIcon.SetHeight(16);
			unlocktangentDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarUnlockTangent - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(unlocktangentIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarUnlockTangent - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(unlocktangentDisabledIcon);
		}

		wxString snapvaluePath = FxStudioApp::GetIconPath(wxT("snapvalue.ico"));
		wxString snapvalueDisabledPath = FxStudioApp::GetIconPath(wxT("snapvalue_disabled.ico"));
		if( FxFileExists(snapvaluePath) && FxFileExists(snapvalueDisabledPath) )
		{
			wxIcon snapvalueIcon = wxIcon(wxIconLocation(snapvaluePath));
			snapvalueIcon.SetHeight(16);
			snapvalueIcon.SetWidth(16);
			wxIcon snapvalueDisabledIcon = wxIcon(wxIconLocation(snapvalueDisabledPath));
			snapvalueDisabledIcon.SetHeight(16);
			snapvalueDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarSnapValue - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(snapvalueIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarSnapValue - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(snapvalueDisabledIcon);
		}

		wxString snaptimePath = FxStudioApp::GetIconPath(wxT("snaptime.ico"));
		wxString snaptimeDisabledPath = FxStudioApp::GetIconPath(wxT("snaptime_disabled.ico"));
		if( FxFileExists(snaptimePath) && FxFileExists(snaptimeDisabledPath) )
		{
			wxIcon snaptimeIcon = wxIcon(wxIconLocation(snaptimePath));
			snaptimeIcon.SetHeight(16);
			snaptimeIcon.SetWidth(16);
			wxIcon snaptimeDisabledIcon = wxIcon(wxIconLocation(snaptimeDisabledPath));
			snaptimeDisabledIcon.SetHeight(16);
			snaptimeDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarSnapTime - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(snaptimeIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarSnapTime - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(snaptimeDisabledIcon);
		}

		wxString fitvaluePath = FxStudioApp::GetIconPath(wxT("fitvalue.ico"));
		wxString fitvalueDisabledPath = FxStudioApp::GetIconPath(wxT("fitvalue_disabled.ico"));
		if( FxFileExists(fitvaluePath) && FxFileExists(fitvalueDisabledPath) )
		{
			wxIcon fitvalueIcon = wxIcon(wxIconLocation(fitvaluePath));
			fitvalueIcon.SetHeight(16);
			fitvalueIcon.SetWidth(16);
			wxIcon fitvalueDisabledIcon = wxIcon(wxIconLocation(fitvalueDisabledPath));
			fitvalueDisabledIcon.SetHeight(16);
			fitvalueDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarFitValue - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(fitvalueIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarFitValue - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(fitvalueDisabledIcon);
		}

		wxString fittimePath = FxStudioApp::GetIconPath(wxT("fittime.ico"));
		wxString fittimeDisabledPath = FxStudioApp::GetIconPath(wxT("fittime_disabled.ico"));
		if( FxFileExists(fittimePath) && FxFileExists(fittimeDisabledPath) )
		{
			wxIcon fittimeIcon = wxIcon(wxIconLocation(fittimePath));
			fittimeIcon.SetHeight(16);
			fittimeIcon.SetWidth(16);
			wxIcon fittimeDisabledIcon = wxIcon(wxIconLocation(fittimeDisabledPath));
			fittimeDisabledIcon.SetHeight(16);
			fittimeDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarFitTime - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(fittimeIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarFitTime - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(fittimeDisabledIcon);
		}

		wxString viewactivePath = FxStudioApp::GetIconPath(wxT("viewactive.ico"));
		wxString viewactiveDisabledPath = FxStudioApp::GetIconPath(wxT("viewactive_disabled.ico"));
		if( FxFileExists(viewactivePath) && FxFileExists(viewactiveDisabledPath) )
		{
			wxIcon viewactiveIcon = wxIcon(wxIconLocation(viewactivePath));
			viewactiveIcon.SetHeight(16);
			viewactiveIcon.SetWidth(16);
			wxIcon viewactiveDisabledIcon = wxIcon(wxIconLocation(viewactiveDisabledPath));
			viewactiveDisabledIcon.SetHeight(16);
			viewactiveDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarViewAllActive - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(viewactiveIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarViewAllActive - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(viewactiveDisabledIcon);
		}

		wxString previouskeyPath = FxStudioApp::GetIconPath(wxT("previouskey.ico"));
		wxString previouskeyDisabledPath = FxStudioApp::GetIconPath(wxT("previouskey_disabled.ico"));
		if( FxFileExists(previouskeyPath) && FxFileExists(previouskeyDisabledPath) )
		{
			wxIcon previouskeyIcon = wxIcon(wxIconLocation(previouskeyPath));
			previouskeyIcon.SetHeight(16);
			previouskeyIcon.SetWidth(16);
			wxIcon previouskeyDisabledIcon = wxIcon(wxIconLocation(previouskeyDisabledPath));
			previouskeyDisabledIcon.SetHeight(16);
			previouskeyDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarPrevKey - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(previouskeyIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarPrevKey - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(previouskeyDisabledIcon);
		}

		wxString nextkeyPath = FxStudioApp::GetIconPath(wxT("nextkey.ico"));
		wxString nextkeyDisabledPath = FxStudioApp::GetIconPath(wxT("nextkey_disabled.ico"));
		if( FxFileExists(nextkeyPath) && FxFileExists(nextkeyDisabledPath) )
		{
			wxIcon nextkeyIcon = wxIcon(wxIconLocation(nextkeyPath));
			nextkeyIcon.SetHeight(16);
			nextkeyIcon.SetWidth(16);
			wxIcon nextkeyDisabledIcon = wxIcon(wxIconLocation(nextkeyDisabledPath));
			nextkeyDisabledIcon.SetHeight(16);
			nextkeyDisabledIcon.SetWidth(16);
			_toolbarButtons[ControlID_CurveWidgetToolbarNextKey - ControlID_CurveWidgetToolbarStart]->SetEnabledBitmap(nextkeyIcon);
			_toolbarButtons[ControlID_CurveWidgetToolbarNextKey - ControlID_CurveWidgetToolbarStart]->SetDisabledBitmap(nextkeyDisabledIcon);
		}
	}
}

void FxCurveWidget::CachePivotKeyStartPos()
{
	// Get the pivot key's time/value and cache it.
	FxSize curveIndex, keyIndex;
	_pAnimController->GetPivotKey(curveIndex, keyIndex);
	if( curveIndex != FxInvalidIndex && keyIndex != FxInvalidIndex )
	{
		FxReal time, value, trash;
		_pAnimController->GetKeyInfo(curveIndex, keyIndex, time, value, trash, trash);
		_startPos.m_x = time;
		_startPos.m_y = value;
	}
}

} // namespace Face

} // namespace OC3Ent
