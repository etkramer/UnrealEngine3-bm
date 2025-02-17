//------------------------------------------------------------------------------
// An audio widget to support viewing of audio in waveform or power 
// spectral density plots.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAudioWidget_H__
#define FxAudioWidget_H__

#include "stdwx.h"

#include "FxPlatform.h"
#include "FxAudio.h"
#include "FxFFT.h"
#include "FxColor.h"
#include "FxTimeManager.h"
#include "FxWidget.h"
#include "FxConsoleVariable.h"
#include "FxStressDetector.h"

namespace OC3Ent
{

namespace Face
{

// An audio view.
class FxAudioWidget : public wxWindow, public FxTimeSubscriber, public FxWidget
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxAudioWidget)
	DECLARE_EVENT_TABLE()

public:
	// Constructor.
	FxAudioWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	// Destructor.
	~FxAudioWidget();

	// Needed to use TimeSubscriber.
	void OnNotifyTimeChange( void );
	FxReal GetMinimumTime( void );
	FxReal GetMaximumTime( void );
	FxInt32 GetPaintPriority( void );

	// Sets the FxAudio pointer.
	void SetAudioPointer( FxAudio* pAudio );

	// FxWidget message handlers.
	virtual void OnAppShutdown( FxWidget* sender );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
							    FxAnim* anim );
	virtual void OnSerializeOptions( FxWidget* sender, wxFileConfig* config, FxBool isLoading );
	virtual void OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, FxPhonWordList* phonemeList ); // *temp*
	virtual void OnRecalculateGrid( FxWidget* sender );
	virtual void OnGestureConfigChanged( FxWidget* sender, FxGestureConfig* config );
	
	// The types of plots that can be drawn in the view.
	enum FxAudioPlotType
	{
		APT_Waveform = 0,
		APT_Spectral
	};
	void SetPlotType( FxAudioPlotType plotType );

	// Forces the options dialog to appear.
	void ShowOptionsDialog( void );

	// The options dialog is a friend to support the "apply" feature.
	friend class FxAudioWidgetOptionsDialog;

	// Callback when a drawing option is updated.
	static void OnDrawOptionUpdated( FxConsoleVariable& cvar );

protected:
	// The aw_showstress console variable.
	FxConsoleVariable* aw_showstress;
	// The digital audio object.
	FxAudio* _pAudio;
	// FxTrue if everything is loaded and ready to draw.  FxFalse otherwise.
	FxBool _readyToDraw;
	// *temp* A pointer to the phoneme list for the animation.
	FxPhonWordList* _phonList;

	// Valid window lengths.  Window lengths must be a power of two.
	enum FxWindowLength
	{
		WL_16 = 4,
		WL_32 = 5,
		WL_64 = 6,
		WL_128 = 7,
		WL_256 = 8,
		WL_512 = 9,
		WL_1024 = 10,
		WL_2048 = 11,
		WL_4096 = 12,
		WL_8192 = 13,
		WL_16384 = 14,
		WL_First = WL_16,
		WL_Last = WL_16384
	};
	// The window length.
	FxInt32 _windowLength;
	// The default window length.
	static const FxWindowLength kDefaultWindowLength;
	// The frame step (always window length divided by two).
	FxInt32 _frameStep;
	// The windowing function type.
	FxFFT::FxFFTWindowType _windowFnType;
	// The default windowing function type.
	static const FxFFT::FxFFTWindowType kDefaultWindowFunctionType;

	// Sets the window length.  Never set it directly without the proper shift
	// and frame step value.
	void _setWindowLength( FxWindowLength windowLength );

	// The power spectral density of the digital audio.
	FxArray< FxArray<FxDReal> > _powerSpectralDensity;

	// The pitch, power and voiced/unvoiced information.
	FxStressDetector _stressDetector;
	// The gesture configuration.
	FxGestureConfig _gestureConfig;

	// The power of the audio at each sample.
	FxArray<FxDReal> _power;
	// The range-normalized power of the audio at each sample.
	FxArray<FxDReal> _normalizedPower;
	// The minimum power value.
	FxDReal _minPower;
	// The maximum power value.
	FxDReal _maxPower;
	// The maximum power spectral density value.
	FxDReal _maxPowerSpectralDensityValue;

	// The gamma correction for the colors (used when computing the power 
	// lookup table).
	FxDReal _colorGamma;

	// The spectral low energy color.
	FxColor _lowEnergyColor;
	// The default spectral low energy color.
	static const FxColor kDefaultLowEnergyColor;
	// The spectral high energy color.
	FxColor _highEnergyColor;
	// The default spectral high energy color.
	static const FxColor kDefaultHighEnergyColor;
	// FxTrue if the color table should be "reversed."
	FxBool _reverseSpectralColors;
	// The default value of reverse spectral colors.
	static const FxBool kDefaultReverseSpectralColors;
	// The default gamma correction factor.
	static const FxDReal kDefaultColorGamma;

	// Power lookup table.
	FxInt32 _powTable[256];

	// Time griding information
	FxArray<FxTimeGridInfo> _timeGridInfos;

	// A color table entry.
	class FxColorTableEntry
	{
	public:
		FxByte red;
		FxByte green;
		FxByte blue;
	};
	// Color lookup table.
	FxColorTableEntry _colorTable[256];

	// Computes the color lookup table.
	void _computeColorTable( void );

	// Look up a color in the color table.
	FxColorTableEntry _colorLookup( FxDReal value );

	// Cleans up the audio widget.
	void _cleanup( void );

	// Computes and caches the power spectral density.
	void _computePowerSpectralDensity( void );

	// Draws the power plot to a dc.
	void _drawPowerPlot( wxDC& dc );
	// Draws the frequency plot
	void _drawFrequencyPlot( wxDC& dc );
	// Draws the emphasis plot
	void _drawEmphasisPlot( wxDC& dc );
	// Draws the section overlay to a dc
	void _drawOverlay( wxDC& dc );
	// Draws the phoneme information plot.
	void _drawVowelInfoPlot( wxDC& dc );

	// The type of plot to draw in the view.
	FxAudioPlotType _plotType;
	// The default plot type to draw.
	static const FxAudioPlotType kDefaultPlotType;

	// The background color for waveform plots.
	FxColor _waveformBackgroundColor;
	// The default background color for waveform plots.
	static const FxColor kDefaultWaveformBackgroundColor;
	// The waveform plot color.
	FxColor _waveformPlotColor;
	// The default waveform plot color.
	static const FxColor kDefaultWaveformPlotColor;
	// The color of the waveform plot grid lines.
	FxColor _waveformGridColor;
	// The default color of the waveform plot grid lines.
	static const FxColor kDefaultWaveformGridColor;
	// The color of the waveform plot 90% threshold lines.
	FxColor _waveformThresholdLineColor;
	// The default color of the waveform plot 90% threshold lines.
	static const FxColor kDefaultWaveformThresholdLineColor;
	// The color of the waveform plot zero axis color.
	FxColor _waveformZeroAxisColor;
	// The default color of the waveform plot zero axis color.
	static const FxColor kDefaultWaveformZeroAxisColor;
	
	// The text color.
	FxColor _textColor;
	// The default text color.
	static const FxColor kDefaultTextColor;
	// The time axis position.
	FxBool _timeAxisOnTop;
	// The default time axis position.
	static const FxBool kDefaultTimeAxisPos;

	// Reset the audio view.
	void OnResetView( wxCommandEvent& event );

	// Toggles the audio plot type.
	void OnToggleFxAudioPlotType( wxCommandEvent& event );
	// Sets the audio plot type to spectral
	void OnSetSpectralPlotType( wxCommandEvent& event );
	// Sets the audio plot type to waveform
	void OnSetWaveformPlotType( wxCommandEvent& event );

	// Shows the options dialog to let users edit options.
	void OnEditOptions( wxCommandEvent& event );

	// Toggles showing the stress
	void OnToggleStress( wxCommandEvent& event );

	// Paint event notification.
	void OnPaint( wxPaintEvent& event );
	// Draw the shaded axes.
	void _drawShadedAxes( wxImage& plotImage, FxInt32 doubleSmallFontPointSize, FxInt32 textLength );
	// Draw the spectral plot.
	void _drawSpectralPlot( wxImage& plotImage );
	// Draw the spectral axis labels.
	void _drawSpectralAxisLabels( wxMemoryDC& dc, FxInt32 doubleSmallFontPointSize );
	// Draw the waveform background grid.
	void _drawWaveformBackgroundGrid( wxMemoryDC& dc );
	// Draw the waveform plot.
	void _drawWaveformPlot( wxMemoryDC& dc );
	// Draw the waveform axis labels.
	void _drawWaveformAxisLabels( wxMemoryDC& dc, FxInt32 doubleSmallFontPointSize );

	// Plays the audio.
	void OnPlayAudio( wxCommandEvent& event );

	// Mouse wheel.
	void OnMouseWheel( wxMouseEvent& event );
	// Left button down.
	void OnLeftButtonDown( wxMouseEvent& event );
	// Left button up.
	void OnLeftButtonUp( wxMouseEvent& event );
	// Mouse movement.
	void OnMouseMove( wxMouseEvent& event );
	// Right button down.
	void OnRightButtonDown( wxMouseEvent& event );
	// Right button up.
	void OnRightButtonUp( wxMouseEvent& event );
	// Middle button down.
	void OnMiddleButtonDown( wxMouseEvent& event );
	// Middle button up.
	void OnMiddleButtonUp( wxMouseEvent& event );

	void OnGetFocus( wxFocusEvent& event );
	void OnLoseFocus( wxFocusEvent& event );

	void OnHelp( wxHelpEvent& event );

	// The minimum time displayed on screen in seconds.
	FxReal _minTime;
	// The maximum time displayed on screen in seconds.
	FxReal _maxTime;

	// The position of the mouse in client rectangle space at the last 
	// captured "frame".
	wxPoint _lastMousePos;

	// The "zoom factor" of the mouse wheel.
	FxReal _mouseWheelZoomFactor;
	// The default "zoom factor" of the mouse wheel.
	static const FxReal kDefaultMouseWheelZoomFactor;

	// Whether or not we have the focus.
	FxBool _hasFocus;

	// Valid zoom directions.
	enum FxZoomDir
	{
		ZD_None = 0,
		ZD_In,
		ZD_Out
	};
	// Zooms the view.  zoomPoint is the "focus" point of the zoom, relative
	// to the client rect.  zoomFactor is the "zoom factor" and zoomDir is 
	// either ZD_In or ZD_Out.
	void _zoom( const wxPoint& zoomPoint, FxReal zoomFactor, FxZoomDir zoomDir );

	// Restores all configurable properties to their default values.
	void _restoreDefaults( void );
};

#ifndef wxCLOSE_BOX
	#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
	#define wxFIXED_MINSIZE 0
#endif

// The options dialog.
class FxAudioWidgetOptionsDialog : public wxDialog
{
	WX_DECLARE_DYNAMIC_CLASS(FxAudioWidgetOptionsDialog)
	DECLARE_EVENT_TABLE()

public:
	// Constructors.
	FxAudioWidgetOptionsDialog();
	FxAudioWidgetOptionsDialog( wxWindow* parent, wxWindowID id = 10005, 
		const wxString& caption = _("Audio View Options"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX );

	// Creation.
	FxBool Create( wxWindow* parent, wxWindowID id = 10005, 
		const wxString& caption = _("Audio View Options"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize(400,300), 
		long style = wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX );

	// Creates the controls and sizers.
	void CreateControls( void );

	// Should we show tooltips?
	static FxBool ShowToolTips( void );

	// Updates the audio view with the current options.
	void SetOptions( void );

protected:
	// Updates the options dialog with the current audio view settings.
	void _getOptions( void );
	// A pointer to the audio view.
	FxAudioWidget* _audioView;
	// Apply button (need a pointer to it to disable / enable it dynamically).
	wxButton* _applyButton;
	// Apply handler.
	void OnApply( wxCommandEvent& event );
	// Restore defaults handler.
	void OnRestoreDefaults( wxCommandEvent& event );
	// Change mouse option handlers.
	void OnMouseWheelZoomFactorChange( wxCommandEvent& event );
	void OnMousePanningSpeedChange( wxCommandEvent& event );
	// Change color handlers.
	void OnGeneralColorChange( wxCommandEvent& event );
	void OnWaveformColorChange( wxCommandEvent& event );
	void OnSpectralColorChange( wxCommandEvent& event );
	void OnGeneralColorsSelectionChange( wxCommandEvent& event );
	void OnWaveformColorsSelectionChange( wxCommandEvent& event );
	void OnSpectralColorsSelectionChange( wxCommandEvent& event );
	void _changeColor( wxColour& colorToChange );
	void OnReverseSpectralChange( wxCommandEvent& event );
	void OnColorGammaChange( wxCommandEvent& event );
	void OnWindowLengthChange( wxCommandEvent& event );
	void OnWindowFunctionChange( wxCommandEvent& event );
	// General handlers.
	void OnTimeAxisOnTopChange( wxCommandEvent& event );

	// Spectral options.
	FxInt32 _windowLength;
	FxInt32 _windowFnType;
	FxBool _reverseSpectralColors;
	FxInt32 _colorGamma;
	wxChoice* _windowLengthChoice;
	wxChoice* _windowFunctionChoice;
	// Spectral colors.
	wxColour _lowEnergyColor;
	wxColour _highEnergyColor;
	// Spectral color list box.
	wxListBox* _spectralColorListBox;
	// Spectral color preview window.
	wxStaticBitmap* _spectralColorPreview;
	// Waveform colors.
	wxColour _waveformBackgroundColor;
	wxColour _waveformPlotColor;
	wxColour _waveformGridColor;
	wxColour _waveformThresholdLineColor;
	wxColour _waveformZeroAxisColor;
	// Waveform color list box.
	wxListBox* _waveformColorListBox;
	// Waveform color preview window.
	wxStaticBitmap* _waveformColorPreview;
	// General options.
	FxInt32 _mouseWheelZoomFactor;
	FxInt32 _mousePanningSpeed;
	// General colors.
	wxColour _textColor;
	// General options.
	FxBool _timeAxisOnTop;
	// General color list box.
	wxListBox* _generalColorListBox;
	// General color preview window.
	wxStaticBitmap* _generalColorPreview;
	// Store the user's custom colors.
	static const FxInt32 _numCustomColors = 16;
	static wxColour _customColors[_numCustomColors];
};

} // namespace Face

} // namespace OC3Ent

#endif

