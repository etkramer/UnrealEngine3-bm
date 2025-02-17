//------------------------------------------------------------------------------
// An options manager for the global settings in FaceFX Studio.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxStudioOptions_H__
#define FxStudioOptions_H__

#include "FxPlatform.h"
#include "FxPhonemeInfo.h"
#include "FxString.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare the session.
class FxStudioSession;

enum FxTimeGridFormat
{
	TGF_Invalid = -1,
	TGF_Seconds = 0,
	TGF_Milliseconds,
	TGF_Frames,
	TGF_Phonemes,
	TGF_Words
};

//------------------------------------------------------------------------------
// FxStudioOptions.
//------------------------------------------------------------------------------
class FxStudioOptions
{
public:
	// Accessors for global options.
	static FxPhoneticAlphabet GetPhoneticAlphabet( void );
	static void SetPhoneticAlphabet( FxPhoneticAlphabet alphabet );

	static wxColour GetMinValueColour( void );
	static void SetMinValueColour( const wxColour& minValColour );

	static wxColour GetZeroValueColour( void );
	static void SetZeroValueColour( const wxColour& zeroValColour );

	static wxColour GetMaxValueColour( void );
	static void SetMaxValueColour( const wxColour& maxValColour );

	static wxColour GetOverlayTextColour( void );
	static void SetOverlayTextColour( const wxColour& overlayColour );

	static wxColour GetFocusedWidgetColour( void );
	static void SetFocusedWidgetColour( const wxColour& focusedWidgetColour );

	static FxTimeGridFormat GetTimeGridFormat( void );
	static void SetTimeGridFormat( FxTimeGridFormat timeGridFormat );

	static FxTimeGridFormat GetTimelineGridFormat( void );
	static void SetTimelineGridFormat( FxTimeGridFormat timelineGridFormat );

	static FxReal GetFrameRate( void );
	static void SetFrameRate( FxReal frameRate );

	static FxSize GetPhonemeBarHeight( void );
	static void SetPhonemeBarHeight( FxSize phonemeBarHeight );

	static wxRect GetMainWndRect( void );
	static void SetMainWndRect( const wxRect& mainWndRect );
	static FxBool GetMainWndMaximized( void );
	static void SetMainWndMaximized( FxBool bIsMainWndMaximized );

	static void GetSplitterPos( FxInt32& mainSplitter, FxInt32& facegraphSplitter, FxInt32& animSplitter );
	static void SetSplitterPos( FxInt32 mainSplitter, FxInt32 facegraphSplitter, FxInt32 animSplitter );

	static const wxString& GetSelectedTab( void );
	static void SetSelectedTab( const wxString& selectedTab );

	static const wxString& GetUnicodeFontName( void );
	static void SetUnicodeFontName( const wxString& unicodeFontName );

	// Resets the options to their defaults.
	static void ResetToDefaults( void );

	// Saves the options to a file.
	static void Save( FxStudioSession* session );
	// Loads the options from a file.
	static void Load( FxStudioSession* session );

protected:
	// The phonetic alphabet.
	static FxPhoneticAlphabet _phoneticAlphabet;
	// The minimum value colour.
	static wxColour _minValueColour;
	// The zero value colour.
	static wxColour _zeroValueColour;
	// The maximum value colour.
	static wxColour _maxValueColour;
	// The colour of any text overlaid on the drawing.
	static wxColour _overlayTextColour;
	// The outline colour of the widget with the focus.
	static wxColour _focusedWidgetColour;
	// The time grid format.
	static FxTimeGridFormat _timeGridFormat;
	// The timeline grid format.
	static FxTimeGridFormat _timelineGridFormat;
	// The number of frames per second in TGF_Frames mode
	static FxReal _frameRate;
	// The height of the phoneme bar, in pixels.
	static FxSize _phonemeBarHeight;
	// The main window rect.
	static wxRect _mainWndRect;
	// Whether or not the main window is maximized.
	static FxBool _bIsMainWndMaximized;
	// The splitter positions.
	static FxInt32 _mainSplitter;
	static FxInt32 _facegraphSplitter;
	static FxInt32 _animSplitter;
	// The currently selected tab in the notebook.
	static wxString _selectedTab;
	// The unicode font
	static wxString _unicodeFontName;
};

//------------------------------------------------------------------------------
// FxStudioOptionsDialog.
//------------------------------------------------------------------------------
class FxStudioOptionsDialog : public wxDialog
{
	typedef wxDialog Super;
	WX_DECLARE_DYNAMIC_CLASS(FxStudioOptionsDialog)
	DECLARE_EVENT_TABLE()

public:
	// Constructors.
	FxStudioOptionsDialog();
	FxStudioOptionsDialog( wxWindow* parent, FxStudioSession* session = NULL,
		wxWindowID id = wxID_DEFAULT, const wxString& title = _("FaceFX Studio Options"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("StudioOptionsDialog") );

	// Part of 2-step dialog creation.
	FxBool Create( wxWindow* parent, wxWindowID id = wxID_DEFAULT, 
		const wxString& title = _("FaceFX Studio Options"),
		const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
		long style = wxDEFAULT_DIALOG_STYLE, const wxString& name = wxT("StudioOptionsDialog") );

	void OnOK( wxCommandEvent& event );
	void OnApply( wxCommandEvent& event );

protected:
	void CreateControls( void );
	void GetOptions( void );
	void SetOptions( void );

	void UpdateFontNameDisplay( void );

	void OnChangeColour( wxCommandEvent& event );
	void OnChooseFont( wxCommandEvent& event );
	void OnRestoreDefault( wxCommandEvent& event );

	FxStudioSession* _session;
	wxStaticBitmap*  _colourPreviewMin;
	wxStaticBitmap*  _colourPreviewZero;
	wxStaticBitmap*  _colourPreviewMax;
	wxStaticBitmap*  _colourPreviewText;
	wxStaticBitmap*  _colourPreviewFocus;
	wxRadioBox*      _phoneticAlphabetRadio;
	wxChoice*		 _timeGridFormatChoice;
	wxChoice*		 _timelineGridFormatChoice;
	wxStaticText*    _unicodeFontNameText;

	FxReal			 _frameRate;
	wxString		 _unicodeFontName;
};

} // namespace Face

} // namespace OC3Ent

#endif
