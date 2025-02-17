//------------------------------------------------------------------------------
// An options manager for the global settings in FaceFX Studio.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxStudioOptions.h"
#include "FxStudioSession.h"
#include "FxColourMapping.h"
#include "FxStudioApp.h"
#include "FxTearoffWindow.h"
#include "FxSplitterWindow.h"
#include "FxValidators.h"
#include "FxMath.h"
#include "FxResource.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/colordlg.h"
	#include "wx/confbase.h"
	#include "wx/fileconf.h"
	#include "wx/font.h"
	#include "wx/fontdlg.h"
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// FxStudioOptions.
//------------------------------------------------------------------------------
#define DEFAULT_PHONETIC_ALPHABET       PA_DEFAULT
#define DEFAULT_MIN_VALUE_COLOUR		wxColour(255,0,0)
#define DEFAULT_ZERO_VALUE_COLOUR		wxColour(255,255,255)
#define DEFAULT_MAX_VALUE_COLOUR		wxColour(0,255,0)
#define DEFAULT_OVERLAY_TEXT_COLOUR		wxColour(0,0,0)
#define DEFAULT_FOCUSED_WIDGET_COLOUR	wxColour(255,0,0)
#define DEFAULT_TIME_GRID_FORMAT		TGF_Seconds
#define DEFAULT_FRAMERATE				24
#define DEFAULT_PHONEME_BAR_HEIGHT		50
#define DEFAULT_UNICODE_FONT_NAME		wxT("Arial Unicode MS")
#define DEFAULT_MAIN_WND_RECT			wxRect(-1, -1, 640, 480)
#define DEFAULT_MAIN_WND_MAXIMIZED      FxFalse
#define DEFAULT_MAIN_SPLITTER			150
#define DEFAULT_FACEGRAPH_SPLITTER		290
#define DEFAULT_ANIM_SPLITTER			200
#define DEFAULT_SELECTED_TAB			wxT("")

// The phonetic alphabet.
FxPhoneticAlphabet FxStudioOptions::_phoneticAlphabet = DEFAULT_PHONETIC_ALPHABET;
// The minimum value colour.
wxColour FxStudioOptions::_minValueColour = DEFAULT_MIN_VALUE_COLOUR;
// The zero value colour.
wxColour FxStudioOptions::_zeroValueColour = DEFAULT_ZERO_VALUE_COLOUR;
// The maximum value colour.
wxColour FxStudioOptions::_maxValueColour = DEFAULT_MAX_VALUE_COLOUR;
// The colour of any text overlaid on the drawing.
wxColour FxStudioOptions::_overlayTextColour = DEFAULT_OVERLAY_TEXT_COLOUR;
// The outline colour of the widget with the focus.
wxColour FxStudioOptions::_focusedWidgetColour = DEFAULT_FOCUSED_WIDGET_COLOUR;
// The time grid format.
FxTimeGridFormat FxStudioOptions::_timeGridFormat = DEFAULT_TIME_GRID_FORMAT;
// The timeline grid format.
FxTimeGridFormat FxStudioOptions::_timelineGridFormat = DEFAULT_TIME_GRID_FORMAT;
// The number of frames per second in TGF_Frames mode
FxReal FxStudioOptions::_frameRate = DEFAULT_FRAMERATE;
// The phoneme bar height
FxSize FxStudioOptions::_phonemeBarHeight = DEFAULT_PHONEME_BAR_HEIGHT;
// The unicode font.
wxString FxStudioOptions::_unicodeFontName = DEFAULT_UNICODE_FONT_NAME;
// The main window's rect
wxRect FxStudioOptions::_mainWndRect = DEFAULT_MAIN_WND_RECT;
FxBool FxStudioOptions::_bIsMainWndMaximized = DEFAULT_MAIN_WND_MAXIMIZED;
FxInt32 FxStudioOptions::_mainSplitter = DEFAULT_MAIN_SPLITTER;
FxInt32 FxStudioOptions::_facegraphSplitter = DEFAULT_FACEGRAPH_SPLITTER;
FxInt32 FxStudioOptions::_animSplitter = DEFAULT_ANIM_SPLITTER;
// The currently selected tab in the notebook.
wxString FxStudioOptions::_selectedTab = DEFAULT_SELECTED_TAB;

// Accessors for global options.
FxPhoneticAlphabet FxStudioOptions::GetPhoneticAlphabet( void )
{
	return _phoneticAlphabet;
}

void FxStudioOptions::SetPhoneticAlphabet( FxPhoneticAlphabet alphabet )
{
	_phoneticAlphabet = alphabet;
}

wxColour FxStudioOptions::GetMinValueColour( void )
{
	return _minValueColour;
}

void FxStudioOptions::SetMinValueColour( const wxColour& minValColour )
{
	_minValueColour = minValColour;
}

wxColour FxStudioOptions::GetZeroValueColour( void )
{
	return _zeroValueColour;
}

void FxStudioOptions::SetZeroValueColour( const wxColour& zeroValColour )
{
	_zeroValueColour = zeroValColour;
}

wxColour FxStudioOptions::GetMaxValueColour( void )
{
	return _maxValueColour;
}

void FxStudioOptions::SetMaxValueColour( const wxColour& maxValColour )
{
	_maxValueColour = maxValColour;
}

wxColour FxStudioOptions::GetOverlayTextColour( void )
{
	return _overlayTextColour;
}

void FxStudioOptions::SetOverlayTextColour( const wxColour& overlayColour )
{
	_overlayTextColour = overlayColour;
}

wxColour FxStudioOptions::GetFocusedWidgetColour( void )
{
	return _focusedWidgetColour;
}

void FxStudioOptions::SetFocusedWidgetColour( const wxColour& focusedWidgetColour )
{
	_focusedWidgetColour = focusedWidgetColour;
}

FxTimeGridFormat FxStudioOptions::GetTimeGridFormat( void )
{
	return _timeGridFormat;
}

void FxStudioOptions::SetTimeGridFormat( FxTimeGridFormat timeGridFormat )
{
	_timeGridFormat = timeGridFormat;
}

FxTimeGridFormat FxStudioOptions::GetTimelineGridFormat( void )
{
	return _timelineGridFormat;
}

void FxStudioOptions::SetTimelineGridFormat( FxTimeGridFormat timelineGridFormat )
{
	_timelineGridFormat = timelineGridFormat;
}

FxReal FxStudioOptions::GetFrameRate( void )
{
	return _frameRate;
}

void FxStudioOptions::SetFrameRate( FxReal frameRate )
{
	_frameRate = frameRate;
}

FxSize FxStudioOptions::GetPhonemeBarHeight( void )
{
	return _phonemeBarHeight;
}

void FxStudioOptions::SetPhonemeBarHeight( FxSize phonemeBarHeight )
{
	_phonemeBarHeight = phonemeBarHeight;
}

const wxString& FxStudioOptions::GetUnicodeFontName( void )
{
	return _unicodeFontName;
}

void FxStudioOptions::SetUnicodeFontName( const wxString& unicodeFontName )
{
	_unicodeFontName = unicodeFontName;
}

wxRect FxStudioOptions::GetMainWndRect( void )
{
	return _mainWndRect;
}

void FxStudioOptions::SetMainWndRect( const wxRect& mainWndRect )
{
	_mainWndRect = mainWndRect;
}

FxBool FxStudioOptions::GetMainWndMaximized( void )
{
	return _bIsMainWndMaximized;
}

void FxStudioOptions::SetMainWndMaximized( FxBool bIsMainWndMaximized )
{
	_bIsMainWndMaximized = bIsMainWndMaximized;
}

void FxStudioOptions::GetSplitterPos( FxInt32& mainSplitter, FxInt32& facegraphSplitter, FxInt32& animSplitter )
{
	mainSplitter      = _mainSplitter;
	facegraphSplitter = _facegraphSplitter;
	animSplitter      = _animSplitter;
}

void FxStudioOptions::SetSplitterPos( FxInt32 mainSplitter, FxInt32 facegraphSplitter, FxInt32 animSplitter )
{
	_mainSplitter      = mainSplitter;
	_facegraphSplitter = facegraphSplitter;
	_animSplitter      = animSplitter;
}

const wxString& FxStudioOptions::GetSelectedTab( void )
{
	return _selectedTab;
}

void FxStudioOptions::SetSelectedTab( const wxString& selectedTab )
{
	_selectedTab = selectedTab;
}

// Resets the options to their defaults.
void FxStudioOptions::ResetToDefaults( void )
{
	_phoneticAlphabet    = DEFAULT_PHONETIC_ALPHABET;
	_minValueColour      = DEFAULT_MIN_VALUE_COLOUR;
	_zeroValueColour     = DEFAULT_ZERO_VALUE_COLOUR;
	_maxValueColour      = DEFAULT_MAX_VALUE_COLOUR;
	_overlayTextColour   = DEFAULT_OVERLAY_TEXT_COLOUR;
	_focusedWidgetColour = DEFAULT_FOCUSED_WIDGET_COLOUR;
	_timeGridFormat		 = DEFAULT_TIME_GRID_FORMAT;
	_timelineGridFormat  = DEFAULT_TIME_GRID_FORMAT;
	_frameRate			 = DEFAULT_FRAMERATE;
	_phonemeBarHeight	 = DEFAULT_PHONEME_BAR_HEIGHT;
	_unicodeFontName     = DEFAULT_UNICODE_FONT_NAME;
}

// Saves the options to a file.
void FxStudioOptions::Save( FxStudioSession* session )
{
	wxFileName filename(FxStudioApp::GetAppDirectory());
	filename.SetName(wxT("FxStudioConfig.ini"));
	wxFileConfig* config = new wxFileConfig(wxT("FaceFX Studio"), wxT("OC3 Entertainment, Inc."), filename.GetFullPath());
	config->Write(wxT("/General/PhoneticAlphabet"), static_cast<long>(_phoneticAlphabet));
	config->Write(wxT("/General/MinValueColour"), FxColourToString(_minValueColour));
	config->Write(wxT("/General/ZeroValueColour"), FxColourToString(_zeroValueColour));
	config->Write(wxT("/General/MaxValueColour"), FxColourToString(_maxValueColour));
	config->Write(wxT("/General/OverlayColour"), FxColourToString(_overlayTextColour));
	config->Write(wxT("/General/FocusedWidgetColour"), FxColourToString(_focusedWidgetColour));
	config->Write(wxT("/General/TimeGridFormat"), static_cast<long>(_timeGridFormat));
	config->Write(wxT("/General/TimelineGridFormat"), static_cast<long>(_timelineGridFormat));
	config->Write(wxT("/General/FrameRate"), _frameRate);
	config->Write(wxT("/General/PhonemeBarHeight"), static_cast<long>(_phonemeBarHeight));
	config->Write(wxT("/General/UnicodeFontName"), _unicodeFontName);

	config->Write(wxT("/MainWnd/Top"),    static_cast<long>(_mainWndRect.GetTop()));
	config->Write(wxT("/MainWnd/Left"),   static_cast<long>(_mainWndRect.GetLeft()));
	config->Write(wxT("/MainWnd/Width"),  static_cast<long>(_mainWndRect.GetWidth()));
	config->Write(wxT("/MainWnd/Height"), static_cast<long>(_mainWndRect.GetHeight()));

	config->Write(wxT("/MainWnd/Maximized"), _bIsMainWndMaximized);

	config->Write(wxT("/MainWnd/MainSplitter"),      static_cast<long>(_mainSplitter));
	config->Write(wxT("/MainWnd/FacegraphSplitter"), static_cast<long>(_facegraphSplitter));
	config->Write(wxT("/MainWnd/AnimSplitter"),      static_cast<long>(_animSplitter));
	
	config->Write(wxT("/MainWnd/SelectedTab"), _selectedTab);

	// Save the tearoff windows.
	FxTearoffWindow::CacheAllCurrentlyTornTearoffWindowStates();
	FxSize numTearoffWindows = FxTearoffWindow::GetNumManagedTearoffWindows();
	for( FxSize i = 0; i < numTearoffWindows; ++i )
	{
		FxTearoffWindow* pWindow = FxTearoffWindow::GetManagedTearoffWindow(i);
		if( pWindow )
		{
			wxString key(wxT("/"));
			key += pWindow->GetTitle();
			key += wxT("/");

			config->Write(key + wxT("IsTorn"), pWindow->IsTorn());
			config->Write(key + wxT("Top"), pWindow->GetPos().y);
			config->Write(key + wxT("Left"), pWindow->GetPos().x);
			config->Write(key + wxT("Width"), pWindow->GetSize().GetWidth());
			config->Write(key + wxT("Height"), pWindow->GetSize().GetHeight());
			config->Write(key + wxT("AlwaysOnTop"), pWindow->GetAlwaysOnTop());
			config->Write(key + wxT("Minimized"), pWindow->GetMinimized());
			config->Write(key + wxT("Maximized"), pWindow->GetMaximized());
		}
	}

	session->OnSerializeOptions(NULL, config, FxFalse);
	delete config;
}

// Loads the options from a file
void FxStudioOptions::Load( FxStudioSession* session )
{
	wxFileName filename(FxStudioApp::GetAppDirectory());
	filename.SetName(wxT("FxStudioConfig.ini"));
	wxFileConfig* config = new wxFileConfig(wxT("FaceFX Studio"), wxT("OC3 Entertainment, Inc."), filename.GetFullPath());
	_phoneticAlphabet = static_cast<FxPhoneticAlphabet>(config->Read(wxT("/General/PhoneticAlphabet"), static_cast<long>(DEFAULT_PHONETIC_ALPHABET)));
	_minValueColour = FxStringToColour(config->Read( wxT("/General/MinValueColour"), FxColourToString(DEFAULT_MIN_VALUE_COLOUR)));
	_zeroValueColour = FxStringToColour(config->Read( wxT("/General/ZeroValueColour"), FxColourToString(DEFAULT_ZERO_VALUE_COLOUR)));
	_maxValueColour = FxStringToColour(config->Read( wxT("/General/MaxValueColour"), FxColourToString(DEFAULT_MAX_VALUE_COLOUR)));
	_overlayTextColour = FxStringToColour(config->Read( wxT("/General/OverlayColour"), FxColourToString(DEFAULT_OVERLAY_TEXT_COLOUR)));
	_focusedWidgetColour = FxStringToColour(config->Read( wxT("/General/FocusedWidgetColour"), FxColourToString(DEFAULT_FOCUSED_WIDGET_COLOUR)));
	_timeGridFormat = static_cast<FxTimeGridFormat>(config->Read(wxT("/General/TimeGridFormat"), static_cast<long>(DEFAULT_TIME_GRID_FORMAT)));
	_timelineGridFormat = static_cast<FxTimeGridFormat>(config->Read(wxT("/General/TimelineGridFormat"), static_cast<long>(DEFAULT_TIME_GRID_FORMAT)));
	_phonemeBarHeight = static_cast<FxSize>(config->Read(wxT("/General/PhonemeBarHeight"), static_cast<long>(DEFAULT_PHONEME_BAR_HEIGHT)));
	_unicodeFontName = config->Read(wxT("/General/UnicodeFontName"), DEFAULT_UNICODE_FONT_NAME);

	if( !FxStudioApp::IsSafeMode() )
	{
		_mainWndRect.y      = config->Read(wxT("/MainWnd/Top"),    static_cast<long>(DEFAULT_MAIN_WND_RECT.GetTop()));
		_mainWndRect.x      = config->Read(wxT("/MainWnd/Left"),   static_cast<long>(DEFAULT_MAIN_WND_RECT.GetLeft()));
		_mainWndRect.width  = config->Read(wxT("/MainWnd/Width"),  static_cast<long>(DEFAULT_MAIN_WND_RECT.GetWidth()));
		_mainWndRect.height = config->Read(wxT("/MainWnd/Height"), static_cast<long>(DEFAULT_MAIN_WND_RECT.GetHeight()));

		config->Read(wxT("/MainWnd/Maximized"), &_bIsMainWndMaximized, DEFAULT_MAIN_WND_MAXIMIZED);

		_mainSplitter      = config->Read(wxT("/MainWnd/MainSplitter"),      static_cast<long>(DEFAULT_MAIN_SPLITTER));
		_facegraphSplitter = config->Read(wxT("/MainWnd/FacegraphSplitter"), static_cast<long>(DEFAULT_FACEGRAPH_SPLITTER));
		_animSplitter      = config->Read(wxT("/MainWnd/AnimSplitter"),      static_cast<long>(DEFAULT_ANIM_SPLITTER));

		_selectedTab       = config->Read(wxT("/MainWnd/SelectedTab"), DEFAULT_SELECTED_TAB);
		
		// Load the tear-off windows.
		FxSize numTearoffWindows = FxTearoffWindow::GetNumManagedTearoffWindows();
		for( FxSize i = 0; i < numTearoffWindows; ++i )
		{
			FxTearoffWindow* pWindow = FxTearoffWindow::GetManagedTearoffWindow(i);
			if( pWindow )
			{
				wxString key(wxT("/"));
				key += pWindow->GetTitle();
				key += wxT("/");

				FxBool isTorn = FxFalse;
				FxBool alwaysOnTop = FxTrue;
				FxBool minimized = FxFalse;
				FxBool maximized = FxFalse;
				config->Read(key + wxT("IsTorn"), &isTorn, FxFalse);
				FxInt32 y = config->Read(key + wxT("Top"), -1);
				FxInt32 x = config->Read(key + wxT("Left"), -1);
				FxInt32 w = config->Read(key + wxT("Width"), -1);
				FxInt32 h = config->Read(key + wxT("Height"), -1);
				config->Read(key + wxT("AlwaysOnTop"), &alwaysOnTop, FxTrue);
				config->Read(key + wxT("Minimized"), &minimized, FxFalse);
				config->Read(key + wxT("Maximized"), &maximized, FxFalse);

				if( isTorn )
				{
					pWindow->SetPos(wxPoint(x,y));
					pWindow->SetSize(wxSize(w,h));

					// Check if the window is in a splitter window.
					FxSize numSplitterWindows = FxSplitterWindow::GetNumManagedSplitterWindows();
					FxBool handledBySplitter = FxFalse;
					for( FxSize j = 0; j < numSplitterWindows; ++j )
					{
						FxSplitterWindow* splitterWindow = FxSplitterWindow::GetManagedSplitterWindow(j);
						if( splitterWindow->GetOriginalTearoffWindow1() == pWindow )
						{
							splitterWindow->Unsplit(splitterWindow->GetOriginalWindow1());
							handledBySplitter = FxTrue;
						}
						else if( splitterWindow->GetOriginalTearoffWindow2() == pWindow )
						{
							splitterWindow->Unsplit(splitterWindow->GetOriginalWindow2());
							handledBySplitter = FxTrue;
						}
					}

					// If it wasn't in a splitter window, tear it from the notebook.
					if( !handledBySplitter )
					{
						pWindow->Tear();
					}

					pWindow->SetPos(wxPoint(x,y));
					pWindow->SetSize(wxSize(w,h));
					pWindow->SetMaximized(maximized);
					pWindow->SetMinimized(minimized);
					pWindow->SetAlwaysOnTop(alwaysOnTop);
				}
			}
		}
	}

	_frameRate = config->Read(wxT("/General/FrameRate"), _frameRate);
	session->OnSerializeOptions(NULL, config, FxTrue);
	delete config;
}

//------------------------------------------------------------------------------
// FxStudioOptionsDialog.
//------------------------------------------------------------------------------
WX_IMPLEMENT_DYNAMIC_CLASS(FxStudioOptionsDialog,wxDialog)

BEGIN_EVENT_TABLE(FxStudioOptionsDialog,wxDialog)
	EVT_BUTTON(ControlID_StudioOptionsDlgApplyButton, FxStudioOptionsDialog::OnApply)
	EVT_BUTTON(ControlID_StudioOptionsDlgOKButton, FxStudioOptionsDialog::OnOK)
	EVT_BUTTON(ControlID_StudioOptionsDlgChangeColourMinButton, FxStudioOptionsDialog::OnChangeColour)
	EVT_BUTTON(ControlID_StudioOptionsDlgChangeColourZeroButton, FxStudioOptionsDialog::OnChangeColour)
	EVT_BUTTON(ControlID_StudioOptionsDlgChangeColourMaxButton, FxStudioOptionsDialog::OnChangeColour)
	EVT_BUTTON(ControlID_StudioOptionsDlgChangeColourTextButton, FxStudioOptionsDialog::OnChangeColour)
	EVT_BUTTON(ControlID_StudioOptionsDlgChangeColourFocusButton, FxStudioOptionsDialog::OnChangeColour)
	EVT_BUTTON(ControlID_StudioOptionsDlgChooseFontButton, FxStudioOptionsDialog::OnChooseFont)
	EVT_BUTTON(ControlID_StudioOptionsDlgRestoreDefaultButton, FxStudioOptionsDialog::OnRestoreDefault)
END_EVENT_TABLE()

FxStudioOptionsDialog::FxStudioOptionsDialog()
{
}

FxStudioOptionsDialog::FxStudioOptionsDialog( wxWindow* parent, FxStudioSession* session,
	wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, 
	long style, const wxString& name )
	: _session(session)
{
	Create(parent, id, title, pos, size, style, name);
}

// Part of 2-step dialog creation
FxBool FxStudioOptionsDialog::Create( wxWindow* parent, wxWindowID id, 
	const wxString& title, const wxPoint& pos, const wxSize& size, 
	long style, const wxString& name )
{
	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create(parent, id, title, pos, size, style, name);

	CreateControls();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return FxTrue;
}

void FxStudioOptionsDialog::OnOK( wxCommandEvent& FxUnused(event) )
{
	SetOptions();
	EndDialog(wxID_OK);
}

void FxStudioOptionsDialog::OnApply( wxCommandEvent& FxUnused(event) )
{
	SetOptions();
}

void FxStudioOptionsDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(boxSizerV);
	this->SetAutoLayout(TRUE);

	// Create the griding selection box.
	wxString gridFormats[5] = {_("Seconds"), _("Milliseconds"), _("Frames"), _("Phonemes"), _("Words")};
	wxString timelineGridFormats[3] = {_("Seconds"), _("Milliseconds"), _("Frames")};

	wxStaticBoxSizer* timeFormatSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Time Grid Format")), wxVERTICAL);
	wxFlexGridSizer* flexGrid = new wxFlexGridSizer(2,2);
	flexGrid->Add(new wxStaticText(this, wxID_ANY, _("Curve editor time format:")), 0, wxALL, 5);
	_timeGridFormatChoice = new wxChoice(this, ControlID_StudioOptionsDlgTimeGridFormatChoice,
		wxDefaultPosition, wxDefaultSize, 5, gridFormats);
	flexGrid->Add(_timeGridFormatChoice, 0, wxALIGN_LEFT|wxALL, 5);

	flexGrid->Add(new wxStaticText(this, wxID_ANY, _("Timeline format:")), 0, wxALL, 5);
	_timelineGridFormatChoice = new wxChoice(this, ControlID_StudioOptionsDlgTimelineGridFormatChoice,
		wxDefaultPosition, wxDefaultSize, 3, timelineGridFormats);
	flexGrid->Add(_timelineGridFormatChoice, 0, wxALIGN_LEFT|wxALL, 5);
	timeFormatSizer->Add(flexGrid, 0, wxGROW, 0);

	wxBoxSizer* timeRow = new wxBoxSizer(wxHORIZONTAL);
	timeRow->Add(new wxStaticText(this, wxID_ANY, _("Frame Rate (fps):")), 0, wxALL, 5);
	timeRow->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_frameRate, wxT("%.0f"))), 0, wxALL, 5);
	timeFormatSizer->Add(timeRow);

	boxSizerV->Add(timeFormatSizer, 0, wxGROW|wxALL, 5);

	// Create the phonetic alphabet selection box.
	wxString choices[3] = {_("FaceFX"), _("SAMPA"), _("IPA")};
	_phoneticAlphabetRadio = new wxRadioBox(this, ControlID_StudioOptionsDlgPhoneticAlphabetRadio, 
		_("Phonetic Alphabet"), wxDefaultPosition, wxDefaultSize, 
		3, choices, 0, wxRA_SPECIFY_COLS);
	boxSizerV->Add(_phoneticAlphabetRadio, 0, wxALIGN_LEFT|wxALL|wxGROW, 5);

	// Create the color section.
	wxBitmap previewBmp(wxNullBitmap);
	
	// Min colour.
	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(row, 0, wxALIGN_LEFT|wxALL, 0);
	_colourPreviewMin = new wxStaticBitmap(this, wxID_DEFAULT,
		previewBmp, wxDefaultPosition, wxSize(70, 20), wxBORDER_SIMPLE);
	row->Add(_colourPreviewMin, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* changeMinColourBtn = new wxButton(this, ControlID_StudioOptionsDlgChangeColourMinButton,
		_("Change Min Value Color..."), wxDefaultPosition, wxSize(155,20), wxBU_LEFT);
	row->Add(changeMinColourBtn, 1, wxALIGN_LEFT|wxALL, 5);
	// Zero colour.
	row = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(row, 0, wxALIGN_LEFT|wxALL, 0);
	_colourPreviewZero = new wxStaticBitmap(this, wxID_DEFAULT,
		previewBmp, wxDefaultPosition, wxSize(70, 20), wxBORDER_SIMPLE);
	row->Add(_colourPreviewZero, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* changeZeroColourBtn = new wxButton(this, ControlID_StudioOptionsDlgChangeColourZeroButton,
		_("Change Zero Color..."), wxDefaultPosition, wxSize(155,20), wxBU_LEFT);
	row->Add(changeZeroColourBtn, 1, wxALIGN_LEFT|wxALL, 5);
	// Max colour.
	row = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(row, 0, wxALIGN_LEFT|wxALL, 0);
	_colourPreviewMax = new wxStaticBitmap(this, wxID_DEFAULT,
		previewBmp, wxDefaultPosition, wxSize(70, 20), wxBORDER_SIMPLE);
	row->Add(_colourPreviewMax, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* changeMaxColourBtn = new wxButton(this, ControlID_StudioOptionsDlgChangeColourMaxButton,
		_("Change Max Value Color..."), wxDefaultPosition, wxSize(155,20), wxBU_LEFT);
	row->Add(changeMaxColourBtn, 1, wxALIGN_LEFT|wxALL, 5);
	// Text colour.
	row = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(row, 0, wxALIGN_LEFT|wxALL, 0);
	_colourPreviewText = new wxStaticBitmap(this, wxID_DEFAULT,
		previewBmp, wxDefaultPosition, wxSize(70, 20), wxBORDER_SIMPLE);
	row->Add(_colourPreviewText, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* changeTextColourBtn = new wxButton(this, ControlID_StudioOptionsDlgChangeColourTextButton,
		_("Change Overlay Color..."), wxDefaultPosition, wxSize(155,20), wxBU_LEFT);
	row->Add( changeTextColourBtn, 1, wxALIGN_LEFT|wxALL, 5 );
	// Focused widget colour.
	row = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(row, 0, wxALIGN_LEFT|wxALL, 0);
	_colourPreviewFocus = new wxStaticBitmap(this, wxID_DEFAULT,
		previewBmp, wxDefaultPosition, wxSize(70, 20), wxBORDER_SIMPLE);
	row->Add(_colourPreviewFocus, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* changeFocusColourBtn = new wxButton(this, ControlID_StudioOptionsDlgChangeColourFocusButton,
		_("Change Widget Outline..."), wxDefaultPosition, wxSize(155,20), wxBU_LEFT);
	row->Add(changeFocusColourBtn, 1, wxALIGN_LEFT|wxALL, 5);

	row = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(row, 0, wxALIGN_LEFT|wxALL, 0);
	wxButton* chooseFontButton = new wxButton(this, ControlID_StudioOptionsDlgChooseFontButton, _("Choose..."));
	row->Add(chooseFontButton, 0, wxALIGN_LEFT|wxALL, 5);
	_unicodeFontNameText = new wxStaticText(this, wxID_ANY, _("Unicode Font: "));
	row->Add(_unicodeFontNameText, 0, wxALIGN_LEFT|wxALL, 5);

	wxButton* restoreDefaults = new wxButton(this, ControlID_StudioOptionsDlgRestoreDefaultButton, _("&Restore Defaults"));
	boxSizerV->Add(restoreDefaults, 0, wxALL|wxGROW, 5);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	boxSizerV->Add(buttonSizer);
	wxButton* okButton = new wxButton(this, ControlID_StudioOptionsDlgOKButton, _("&OK"));
	buttonSizer->Add(okButton, 0, wxALL|wxALIGN_LEFT, 5);
	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("&Cancel"));
	buttonSizer->Add(cancelButton, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	wxButton* applyButton = new wxButton(this, ControlID_StudioOptionsDlgApplyButton, _("&Apply"));
	buttonSizer->Add(applyButton, 0, wxALL|wxALIGN_RIGHT, 5);

	GetOptions();
}

void FxStudioOptionsDialog::GetOptions( void )
{
	_colourPreviewMin->SetBackgroundColour(FxStudioOptions::GetMinValueColour());
	_colourPreviewZero->SetBackgroundColour(FxStudioOptions::GetZeroValueColour());
	_colourPreviewMax->SetBackgroundColour(FxStudioOptions::GetMaxValueColour());
	_colourPreviewText->SetBackgroundColour(FxStudioOptions::GetOverlayTextColour());
	_colourPreviewFocus->SetBackgroundColour(FxStudioOptions::GetFocusedWidgetColour());
	_phoneticAlphabetRadio->SetSelection(FxStudioOptions::GetPhoneticAlphabet());
	_timeGridFormatChoice->SetSelection(FxStudioOptions::GetTimeGridFormat());
	_timelineGridFormatChoice->SetSelection(FxStudioOptions::GetTimelineGridFormat());
	_frameRate = FxStudioOptions::GetFrameRate();
	_unicodeFontName = FxStudioOptions::GetUnicodeFontName();
	UpdateFontNameDisplay();
	TransferDataToWindow();
	Refresh(FALSE);
}

void FxStudioOptionsDialog::SetOptions( void )
{
	TransferDataFromWindow();
	FxStudioOptions::SetMinValueColour(_colourPreviewMin->GetBackgroundColour());
	FxStudioOptions::SetZeroValueColour(_colourPreviewZero->GetBackgroundColour());
	FxStudioOptions::SetMaxValueColour(_colourPreviewMax->GetBackgroundColour());
	FxStudioOptions::SetOverlayTextColour(_colourPreviewText->GetBackgroundColour());
	FxStudioOptions::SetFocusedWidgetColour(_colourPreviewFocus->GetBackgroundColour());
	FxStudioOptions::SetPhoneticAlphabet(static_cast<FxPhoneticAlphabet>(_phoneticAlphabetRadio->GetSelection()));
	FxStudioOptions::SetTimeGridFormat(static_cast<FxTimeGridFormat>(_timeGridFormatChoice->GetSelection()));
	FxStudioOptions::SetTimelineGridFormat(static_cast<FxTimeGridFormat>(_timelineGridFormatChoice->GetSelection()));
	_frameRate = FxAbs(_frameRate);
	FxStudioOptions::SetFrameRate(FxAbs(_frameRate));
	FxStudioOptions::SetUnicodeFontName(_unicodeFontName);
	if( _session )
	{
		_session->OnRecalculateGrid(NULL);
		_session->OnRefresh(NULL);
	}
	TransferDataToWindow();
}

void FxStudioOptionsDialog::UpdateFontNameDisplay( void )
{
	_unicodeFontNameText->SetLabel(wxString::Format(_("Unicode Font: %s"), _unicodeFontName));
}

void FxStudioOptionsDialog::OnChangeColour( wxCommandEvent& event )
{
	wxColour currentColour;
	switch( event.GetId() )
	{
	case ControlID_StudioOptionsDlgChangeColourMinButton:
		currentColour = _colourPreviewMin->GetBackgroundColour();
		break;
	default:
	case ControlID_StudioOptionsDlgChangeColourZeroButton:
		currentColour = _colourPreviewZero->GetBackgroundColour();
		break;
	case ControlID_StudioOptionsDlgChangeColourMaxButton:
		currentColour = _colourPreviewMax->GetBackgroundColour();
		break;
	case ControlID_StudioOptionsDlgChangeColourTextButton:
		currentColour = _colourPreviewText->GetBackgroundColour();
		break;
	case ControlID_StudioOptionsDlgChangeColourFocusButton:
		currentColour = _colourPreviewFocus->GetBackgroundColour();
		break;
	}

	wxColourDialog colorPickerDialog(this);
	colorPickerDialog.GetColourData().SetChooseFull(true);
	colorPickerDialog.GetColourData().SetColour(currentColour);
	// Remember the user's custom colors.
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		colorPickerDialog.GetColourData().SetCustomColour(i, FxColourMap::GetCustomColours()[i]);
	}
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( colorPickerDialog.ShowModal() == wxID_OK )
	{
		currentColour = colorPickerDialog.GetColourData().GetColour();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		FxColourMap::GetCustomColours()[i] = colorPickerDialog.GetColourData().GetCustomColour(i);
	}

	switch( event.GetId() )
	{
	case ControlID_StudioOptionsDlgChangeColourMinButton:
		_colourPreviewMin->SetBackgroundColour(currentColour);
		break;
	default:
	case ControlID_StudioOptionsDlgChangeColourZeroButton:
		_colourPreviewZero->SetBackgroundColour(currentColour);
		break;
	case ControlID_StudioOptionsDlgChangeColourMaxButton:
		_colourPreviewMax->SetBackgroundColour(currentColour);
		break;
	case ControlID_StudioOptionsDlgChangeColourTextButton:
		_colourPreviewText->SetBackgroundColour(currentColour);
		break;
	case ControlID_StudioOptionsDlgChangeColourFocusButton:
		_colourPreviewFocus->SetBackgroundColour(currentColour);
		break;
	}
	Refresh(FALSE);
}

void FxStudioOptionsDialog::OnChooseFont( wxCommandEvent& FxUnused(event) )
{
	wxFont initialFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, _unicodeFontName, wxFONTENCODING_SYSTEM);
	wxFont font = wxGetFontFromUser(this, initialFont);
	if( font.Ok() )
	{
		_unicodeFontName = font.GetFaceName();
		UpdateFontNameDisplay();
	}
}

void FxStudioOptionsDialog::OnRestoreDefault( wxCommandEvent& FxUnused(event) )
{
	_colourPreviewMin->SetBackgroundColour(DEFAULT_MIN_VALUE_COLOUR);
	_colourPreviewZero->SetBackgroundColour(DEFAULT_ZERO_VALUE_COLOUR);
	_colourPreviewMax->SetBackgroundColour(DEFAULT_MAX_VALUE_COLOUR);
	_colourPreviewText->SetBackgroundColour(DEFAULT_OVERLAY_TEXT_COLOUR);
	_colourPreviewFocus->SetBackgroundColour(DEFAULT_FOCUSED_WIDGET_COLOUR);
	_phoneticAlphabetRadio->SetSelection(DEFAULT_PHONETIC_ALPHABET);
	_timeGridFormatChoice->SetSelection(DEFAULT_TIME_GRID_FORMAT);
	_timelineGridFormatChoice->SetSelection(DEFAULT_TIME_GRID_FORMAT);
	_frameRate = DEFAULT_FRAMERATE;
	_unicodeFontName = DEFAULT_UNICODE_FONT_NAME;
	UpdateFontNameDisplay();
	TransferDataToWindow();
	Refresh(FALSE);
}

} // namespace Face

} // namespace OC3Ent
