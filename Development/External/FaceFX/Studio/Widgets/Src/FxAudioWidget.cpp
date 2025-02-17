//------------------------------------------------------------------------------
// An audio widget to support viewing of audio in waveform or power 
// spectral density plots.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxStudioApp.h"
#include "FxConsole.h"
#include "FxVec3.h"
#include "FxMath.h"
#include "FxAudioWidget.h"
#include "FxAnimUserData.h"
#include "FxColourMapping.h"
#include "FxStudioOptions.h"
#include "FxVM.h"
#include "FxStudioAnimPlayer.h"
#include "FxTearoffWindow.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/image.h"
	#include "wx/notebook.h"
	#include "wx/valgen.h"
	#include "wx/colordlg.h"
	#include "wx/confbase.h"
	#include "wx/fileconf.h"
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// The audio view.
//------------------------------------------------------------------------------

// The default configurable property values.
const FxAudioWidget::FxWindowLength 
	  FxAudioWidget::kDefaultWindowLength = FxAudioWidget::WL_512;
const FxFFT::FxFFTWindowType 
	  FxAudioWidget::kDefaultWindowFunctionType = FxFFT::WT_Blackman;
const FxColor FxAudioWidget::kDefaultLowEnergyColor = 
	  FxColor(FX_BLACK);
const FxColor FxAudioWidget::kDefaultHighEnergyColor =
	  FxColor(FX_RGB(245,250,255));
const FxBool FxAudioWidget::kDefaultReverseSpectralColors = FxFalse;
const FxDReal FxAudioWidget::kDefaultColorGamma = 1.75;
const FxAudioWidget::FxAudioPlotType
      FxAudioWidget::kDefaultPlotType = FxAudioWidget::APT_Spectral;
const FxColor FxAudioWidget::kDefaultWaveformBackgroundColor = 
	  FxColor(FX_RGB(255,255,220));
const FxColor FxAudioWidget::kDefaultWaveformPlotColor = 
	  FxColor(FX_RGB(70,70,70));
const FxColor FxAudioWidget::kDefaultWaveformGridColor = 
      FxColor(FX_RGB(198,198,255));
const FxColor FxAudioWidget::kDefaultWaveformThresholdLineColor = 
	  FxColor(FX_RGB(150,0,0));
const FxColor FxAudioWidget::kDefaultWaveformZeroAxisColor = 
      FxColor(FX_RGB(150,0,0));
const FxColor FxAudioWidget::kDefaultTextColor = 
      FxColor(FX_WHITE);
const FxBool FxAudioWidget::kDefaultTimeAxisPos = FxTrue;
const FxReal FxAudioWidget::kDefaultMouseWheelZoomFactor = 1.5f;

WX_IMPLEMENT_DYNAMIC_CLASS( FxAudioWidget, wxWindow )

BEGIN_EVENT_TABLE( FxAudioWidget, wxWindow )
	EVT_MENU( MenuID_AudioWidgetResetView, FxAudioWidget::OnResetView )
	EVT_MENU(MenuID_AudioWidgetTogglePlotType, FxAudioWidget::OnToggleFxAudioPlotType)
	EVT_MENU(MenuID_AudioWidgetSpectralPlot, FxAudioWidget::OnSetSpectralPlotType)
	EVT_MENU(MenuID_AudioWidgetWaveformPlot, FxAudioWidget::OnSetWaveformPlotType)
	EVT_MENU( MenuID_AudioWidgetEditOptions, FxAudioWidget::OnEditOptions )
	EVT_MENU( MenuID_AudioWidgetPlayAudio, FxAudioWidget::OnPlayAudio )
	EVT_MENU( MenuID_AudioWidgetToggleStress, FxAudioWidget::OnToggleStress )
	EVT_PAINT( FxAudioWidget::OnPaint )
	EVT_MOUSEWHEEL( FxAudioWidget::OnMouseWheel )
	EVT_LEFT_DOWN( FxAudioWidget::OnLeftButtonDown )
	EVT_LEFT_UP( FxAudioWidget::OnLeftButtonUp )
	EVT_MOTION( FxAudioWidget::OnMouseMove )
	EVT_RIGHT_DOWN( FxAudioWidget::OnRightButtonDown )
	EVT_RIGHT_UP( FxAudioWidget::OnRightButtonUp )
	EVT_MIDDLE_DOWN( FxAudioWidget::OnMiddleButtonDown )
	EVT_MIDDLE_UP( FxAudioWidget::OnMiddleButtonUp )
	EVT_SET_FOCUS( FxAudioWidget::OnGetFocus )
	EVT_KILL_FOCUS( FxAudioWidget::OnLoseFocus )
	EVT_HELP_RANGE( wxID_ANY, wxID_HIGHEST, FxAudioWidget::OnHelp)
END_EVENT_TABLE()

FxAudioWidget::FxAudioWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: Super( parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE )
	, FxWidget( mediator )
	, aw_showstress(FxVM::FindConsoleVariable("aw_showstress"))
	, _pAudio(NULL)
	, _readyToDraw(FxFalse)
	, _maxPowerSpectralDensityValue(0.0)
	, _minTime(0.0f)
	, _maxTime(1.0f)
{
	FxAssert(aw_showstress);
	SetBackgroundColour( wxColour( 192, 192, 192 ) );
	// Set up the keyboard shortcuts.
	static const FxInt32 numEntries = 5;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set( wxACCEL_CTRL, (FxInt32)'R',	MenuID_AudioWidgetResetView );
	entries[1].Set( wxACCEL_CTRL, (FxInt32)'T',	MenuID_AudioWidgetTogglePlotType );
	entries[2].Set( wxACCEL_CTRL, (FxInt32)'P',	MenuID_AudioWidgetEditOptions );
	entries[3].Set( wxACCEL_NORMAL, WXK_SPACE,	MenuID_AudioWidgetPlayAudio );
	entries[4].Set( wxACCEL_NORMAL, WXK_F7,		MenuID_AudioWidgetToggleStress );
	wxAcceleratorTable accelerator( numEntries, entries );
	SetAcceleratorTable( accelerator );
	// Set the configurable properties to their default values.
	_restoreDefaults();
	// Compute the color lookup table.
	_computeColorTable();
	FxTimeManager::RegisterSubscriber( this );
	FxTimeManager::RequestTimeChange( _minTime, _maxTime );
}

FxAudioWidget::~FxAudioWidget()
{
	_cleanup();
}

void FxAudioWidget::OnNotifyTimeChange( void )
{
	FxTimeManager::GetTimes( _minTime, _maxTime );
	Refresh( FALSE );
	Update();
}

FxReal FxAudioWidget::GetMinimumTime( void )
{
	return 0.0f;
}

FxReal FxAudioWidget::GetMaximumTime( void )
{
	FxReal retval = 0.0f;
	if( _pAudio )
	{
		retval = static_cast<FxReal>( _pAudio->GetDuration() );
	}
	return retval;
}

FxInt32 FxAudioWidget::GetPaintPriority( void )
{
	return 4;
}

void FxAudioWidget::SetAudioPointer( FxAudio* pAudio )
{
	_cleanup();
	_pAudio = pAudio;

	_computePowerSpectralDensity();

	if( _pAudio )
	{
		// Compute the frequency
		_stressDetector.Initialize(pAudio->GetSamples(), pAudio->GetNumSamples(), pAudio->GetSampleRate(), 0.025f, 0.005f);
		_stressDetector.Process();
	}

	if( _pAudio )
	{
		FxTimeManager::RequestTimeChange( 0.0f, _pAudio->GetDuration() );
	}
	_readyToDraw = FxTrue;
	Refresh();
}

void FxAudioWidget::OnAppShutdown( FxWidget* FxUnused(sender) )
{
	FxTimeManager::UnregisterSubscriber(this);
	_readyToDraw = FxFalse;
}

void FxAudioWidget::
OnAnimChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim )
{
	if( anim )
	{
		if( anim->GetUserData() )
		{
			FxAnimUserData* userData = static_cast<FxAnimUserData*>(anim->GetUserData());
			FxAudio* newpAudio = userData->GetAudioPointer();
			if( newpAudio != _pAudio )
			{
				SetAudioPointer(newpAudio);
			}
		}
	}
	else
	{
		SetAudioPointer(NULL);
		_readyToDraw = FxFalse;
	}
}

void FxAudioWidget::
OnSerializeOptions( FxWidget* FxUnused(sender), wxFileConfig* config, FxBool isLoading )
{
	if( isLoading )
	{
		_lowEnergyColor = FxStringToColor(config->Read(wxT("/AudioWidget/LowEnergyColor"), FxColorToString(kDefaultLowEnergyColor)));
		_highEnergyColor = FxStringToColor(config->Read(wxT("/AudioWidget/HighEnergyColor"), FxColorToString(kDefaultHighEnergyColor)));
		_waveformBackgroundColor = FxStringToColor(config->Read(wxT("/AudioWidget/WaveformBackgroundColor"), FxColorToString(kDefaultWaveformBackgroundColor)));
		_waveformPlotColor = FxStringToColor(config->Read(wxT("/AudioWidget/WaveformPlotColor"), FxColorToString(kDefaultWaveformPlotColor)));
		_waveformGridColor = FxStringToColor(config->Read(wxT("/AudioWidget/WaveformGridColor"), FxColorToString(kDefaultWaveformGridColor)));
		_waveformThresholdLineColor = FxStringToColor(config->Read(wxT("/AudioWidget/WaveformThresholdLineColor"), FxColorToString(kDefaultWaveformThresholdLineColor)));
		_waveformZeroAxisColor = FxStringToColor(config->Read(wxT("/AudioWidget/WaveformZeroAxisColor"), FxColorToString(kDefaultWaveformZeroAxisColor)));
		_textColor = FxStringToColor(config->Read(wxT("/AudioWidget/TextColor"), FxColorToString(kDefaultTextColor)));
		FxInt32 windowLength; config->Read(wxT("/AudioWidget/WindowLength"), &windowLength, kDefaultWindowLength);
		_setWindowLength(static_cast<FxWindowLength>(windowLength));
		FxInt32 defaultWindowFnType = static_cast<FxInt32>(kDefaultWindowFunctionType);
		FxInt32 windowFnType; config->Read(wxT("/AudioWidget/WindowFunctionType"), &windowFnType, defaultWindowFnType);
		_windowFnType = static_cast<FxFFT::FxFFTWindowType>(windowFnType);
		config->Read(wxT("/AudioWidget/ReverseSpectralColors"), &_reverseSpectralColors, kDefaultReverseSpectralColors);
		config->Read(wxT("/AudioWidget/ColorGamma"), &_colorGamma, kDefaultColorGamma);
		FxInt32 defaultAudioPlotType = static_cast<FxInt32>(kDefaultPlotType);
		FxInt32 plotType; config->Read(wxT("AudioWidget/AudioPlotType"), &plotType, defaultAudioPlotType);
		_plotType = static_cast<FxAudioPlotType>(plotType);
		config->Read(wxT("/AudioWidget/TimeAxisOnTop"), &_timeAxisOnTop, kDefaultTimeAxisPos);
		FxDReal defaultMouseWheelZoomFactor = static_cast<FxDReal>(kDefaultMouseWheelZoomFactor);
		FxDReal mouseWheelZoomFactor = static_cast<FxDReal>(_mouseWheelZoomFactor);
		config->Read(wxT("/AudioWidget/MouseWheelZoomFactor"), &mouseWheelZoomFactor, defaultMouseWheelZoomFactor);
		_mouseWheelZoomFactor = static_cast<FxReal>(mouseWheelZoomFactor);

		// Compute the color lookup table.
		_computeColorTable();
	}
	else
	{
		config->Write(wxT("/AudioWidget/LowEnergyColor"), FxColorToString(_lowEnergyColor));
		config->Write(wxT("/AudioWidget/HighEnergyColor"), FxColorToString(_highEnergyColor));
		config->Write(wxT("/AudioWidget/WaveformBackgroundColor"), FxColorToString(_waveformBackgroundColor));
		config->Write(wxT("/AudioWidget/WaveformPlotColor"), FxColorToString(_waveformPlotColor));
		config->Write(wxT("/AudioWidget/WaveformGridColor"), FxColorToString(_waveformGridColor));
		config->Write(wxT("/AudioWidget/WaveformThresholdLineColor"), FxColorToString(_waveformThresholdLineColor));
		config->Write(wxT("/AudioWidget/WaveformZeroAxisColor"), FxColorToString(_waveformZeroAxisColor));
		config->Write(wxT("/AudioWidget/TextColor"), FxColorToString(_textColor));
		// Convert _windowLength back to an enum index.
		FxInt32 windowLength;
		switch( _windowLength )
		{
		case 16:
			windowLength = 4;
			break;
		case 32:
			windowLength = 5;
			break;
		case 64:
			windowLength = 6;
			break;
		case 128:
			windowLength = 7;
			break;
		case 256:
			windowLength = 8;
			break;
		default:
		case 512:
			windowLength = 9;
			break;
		case 1024:
			windowLength = 10;
			break;
		case 2048:
			windowLength = 11;
			break;
		case 4096:
			windowLength = 12;
			break;
		case 8192:
			windowLength = 13;
			break;
		case 16384:
			windowLength = 14;
			break;
		}
		config->Write(wxT("/AudioWidget/WindowLength"), windowLength);
		FxInt32 windowFnType = static_cast<FxInt32>(_windowFnType);
		config->Write(wxT("/AudioWidget/WindowFunctionType"), windowFnType);
		config->Write(wxT("/AudioWidget/ReverseSpectralColors"), _reverseSpectralColors);
		config->Write(wxT("/AudioWidget/ColorGamma"), static_cast<FxDReal>(_colorGamma));
		FxInt32 plotType = static_cast<FxInt32>(_plotType);
		config->Write(wxT("AudioWidget/AudioPlotType"), plotType);
		config->Write(wxT("/AudioWidget/TimeAxisOnTop"), _timeAxisOnTop);
		config->Write(wxT("/AudioWidget/MouseWheelZoomFactor"), static_cast<FxDReal>(_mouseWheelZoomFactor));
	}
}

void FxAudioWidget::OnAnimPhonemeListChanged( FxWidget* FxUnused(sender), FxAnim* FxUnused(anim), FxPhonWordList* phonemeList )
{
	_phonList = phonemeList;
	if( phonemeList )
	{
		FxPhonemeList genericPhonList;
		phonemeList->ToGenericFormat(genericPhonList);
		_stressDetector.Synch(&genericPhonList, _gestureConfig);
	}
	Refresh(FALSE);
}

void FxAudioWidget::OnRecalculateGrid( FxWidget* FxUnused(sender) )
{
	Refresh(FALSE);
}

void FxAudioWidget::OnGestureConfigChanged( FxWidget* FxUnused(sender), FxGestureConfig* config )
{
	if( config )
	{
		_gestureConfig = (*config);
	}
}

void FxAudioWidget::SetPlotType( FxAudioPlotType plotType )
{
	_plotType = plotType;
	Refresh(FALSE);
}

void FxAudioWidget::ShowOptionsDialog( void )
{
	FxAudioWidgetOptionsDialog editOptionsDialog;
	editOptionsDialog.Create(this);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( editOptionsDialog.ShowModal() == wxID_OK )
	{
		editOptionsDialog.SetOptions();		
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxAudioWidget::OnDrawOptionUpdated( FxConsoleVariable& FxUnused(cvar) )
{
	// Ugly hack to force the audio widget to refresh.
	FxReal minTime, maxTime;
	FxTimeManager::GetTimes(minTime, maxTime);
	FxTimeManager::RequestTimeChange(minTime, maxTime);
}

void FxAudioWidget::_setWindowLength( FxWindowLength windowLength )
{
	_windowLength = 1<<windowLength;
	_frameStep    = _windowLength / 2;
}

void FxAudioWidget::_computeColorTable( void )
{
	// Compute the power lookup table with a gamma correction factor.
	for( FxInt32 i = 0; i < 256; ++i )
	{
		_powTable[i] = static_cast<FxInt32>(255.0 * 
			FxPow(static_cast<FxDReal>(i) / 255.0, _colorGamma));
	}

	// Smoothly interpolate colors from zero to one through the
	// HLS color space.
	for( FxInt32 i = 0; i < 256; ++i )
	{
		FxReal pct = static_cast<FxReal>(i) / 255.0f;
		FxVec3 hls;
		FxVec3 hls1( _highEnergyColor.GetHue(), 
			_highEnergyColor.GetLuminance(), 
			_highEnergyColor.GetSaturation() );

		if( _reverseSpectralColors )
		{
			FxVec3 hls0( 360.0f - _lowEnergyColor.GetHue(), 
				_lowEnergyColor.GetLuminance(), 
				_lowEnergyColor.GetSaturation() );
			if( hls0.x == FxColor::kUndefinedHue )
			{
				hls0.x = hls1.x;
			}
			else if( hls1.x == FxColor::kUndefinedHue )
			{
				hls1.x = hls0.x;
			}
			hls = hls0.Lerp( hls1, pct );
			hls += hls0;
			if( hls.x > 360.0f )
			{
				hls.x -= 360.0f;
			}
		}
		else
		{
			FxVec3 hls0( _lowEnergyColor.GetHue(), 
				_lowEnergyColor.GetLuminance(), 
				_lowEnergyColor.GetSaturation() );
			if( hls0.x == FxColor::kUndefinedHue )
			{
				hls0.x = hls1.x;
			}
			else if( hls1.x == FxColor::kUndefinedHue )
			{
				hls1.x = hls0.x;
			}
			hls = hls0.Lerp( hls1, pct );
			hls += hls0;
		}
		FxColor currentColor( hls.x, hls.y, hls.z );

		_colorTable[i].red   = _powTable[currentColor.GetRedByte()];
		_colorTable[i].green = _powTable[currentColor.GetGreenByte()];
		_colorTable[i].blue  = _powTable[currentColor.GetBlueByte()];
	}
}

FxAudioWidget::FxColorTableEntry 
FxAudioWidget::_colorLookup( FxDReal value )
{
	FxInt32 val = (FxMax<FxDReal>(FxMin<FxDReal>(value, _maxPowerSpectralDensityValue), 0.0) / 
		_maxPowerSpectralDensityValue) * 255;
	return _colorTable[_powTable[val]];
}

void FxAudioWidget::_cleanup( void )
{
	_readyToDraw  = FxFalse;
	_pAudio = NULL;
	_powerSpectralDensity.Clear();
}

void FxAudioWidget::_computePowerSpectralDensity( void )
{
	if( _pAudio )
	{
		// Reset the maximum power spectral density value.
		_maxPowerSpectralDensityValue = 0.0;
		// Clear out any previous results.
		_powerSpectralDensity.Clear();
		// Compute the FFT.
		FxFFT fft(_windowLength, _pAudio->GetSampleRate());
		fft.SetWindowFunction( _windowFnType );
		FxInt32 numFrames = _pAudio->GetNumSamples() / _frameStep;
		FxDReal* frameBuffer = new FxDReal[_windowLength];
		// Compute the FFT and power spectral density for each frame of audio.
		_powerSpectralDensity.Reserve( numFrames );
		for( FxInt32 frame = 0; frame < numFrames; ++frame )
		{
			FxInt32 sampleIndex = frame * _frameStep;
			// Don't step past the end of the audio buffer!
			FxInt32 thisFrameLength = FxMin<FxInt32>(_windowLength, _pAudio->GetNumSamples() - sampleIndex);
			// Fill in the frame.
			for( FxInt32 i = 0; i < thisFrameLength; ++i )
			{
				frameBuffer[i] = _pAudio->GetSample(sampleIndex + i);
			}
			// Fill rest with silence.
			if( thisFrameLength < _windowLength )
			{
				for( FxInt32 i = thisFrameLength; i < _windowLength; ++i )
				{
					frameBuffer[i] = 0.0;
				}
			}

			// Give the new frame to the FFT.
			fft.CopySamples( frameBuffer, _windowLength );

			// Compute the FFT.
			fft.ForwardTransform();

			// Cache the power spectral density for this frame and remember
			// the maximum value seen so far.
			FxArray<FxDReal> column;
			column.Reserve( _frameStep );
			for( FxInt32 i = 0; i < fft.GetNumResults(); ++i )
			{
				FxDReal psd = fft.GetIntensity(i);
				_maxPowerSpectralDensityValue = FxMax<FxDReal>(_maxPowerSpectralDensityValue, psd);
				column.PushBack( psd == 0.0 ? 0.0 : FxLog(psd) );
			}
			_powerSpectralDensity.PushBack(column);
		}
		_maxPowerSpectralDensityValue = FxLog(_maxPowerSpectralDensityValue);
		delete [] frameBuffer;
		frameBuffer = NULL;
	}
}

void FxAudioWidget::_drawPowerPlot( wxDC& dc )
{
	if( _pAudio && _readyToDraw )
	{
		FxReal maxPower = _stressDetector.GetMaxPower();
		FxReal minPower = _stressDetector.GetMinPower();
		wxRect clientRect = GetClientRect();

		for( FxInt32 x = 0; x < clientRect.GetWidth()-1; ++x )
		{
			FxReal time = FxTimeManager::CoordToTime(x, clientRect);
			FxReal nextTime = FxTimeManager::CoordToTime(x+1, clientRect);
			if(_stressDetector.IsVoiced(time)&&_stressDetector.IsVoiced(nextTime))
			{
				FxReal power = _stressDetector.GetPower(time);
				FxReal nextPower = _stressDetector.GetPower(nextTime);
				wxPoint thisPoint(x, clientRect.GetHeight() - (((power-minPower)/(maxPower-minPower)) * clientRect.GetHeight()));
				wxPoint nextPoint(x+1, clientRect.GetHeight() - (((nextPower-minPower)/(maxPower-minPower)) * clientRect.GetHeight()));
				dc.SetPen(wxPen(wxColour(255,0,0), 1, wxSOLID));
				dc.DrawLine(thisPoint, nextPoint);

//				FxReal avgPower = _stressDetector.GetAvgPower(time);
//				FxReal nextAvgPower = _stressDetector.GetAvgPower(nextTime);
//				thisPoint = wxPoint(x, clientRect.GetHeight() - (((avgPower-minPower)/(maxPower-minPower)) * clientRect.GetHeight()));
//				nextPoint = wxPoint(x+1, clientRect.GetHeight() - (((nextAvgPower-minPower)/(maxPower-minPower)) * clientRect.GetHeight()));
//				dc.SetPen(wxPen(wxColour(128,192,0), 1, wxSOLID));
//				dc.DrawLine(thisPoint, nextPoint);

//				FxReal phonPower = _stressDetector.GetPhonemePower(time);
//				FxReal nextPhonPower = _stressDetector.GetPhonemePower(nextTime);
//				thisPoint = wxPoint(x, clientRect.GetHeight() - (((phonPower-minPower)/(maxPower-minPower)) * clientRect.GetHeight()));
//				nextPoint = wxPoint(x+1, clientRect.GetHeight() - (((nextPhonPower-minPower)/(maxPower-minPower)) * clientRect.GetHeight()));
//				dc.SetPen(wxPen(wxColour(0,255,0), 1, wxSOLID));
//				dc.DrawLine(thisPoint, nextPoint);

			}
		}
	}
}

void FxAudioWidget::_drawFrequencyPlot( wxDC& dc )
{
	if( _pAudio && _readyToDraw )
	{
		FxReal maxFrequency = _pAudio->GetSampleRate() / 2;
		wxRect clientRect = GetClientRect();

		dc.SetPen(wxPen(wxColour(255,255,0), 1, wxSOLID));
		for( FxInt32 x = 0; x < clientRect.GetWidth(); ++x )
		{
			FxReal time = FxTimeManager::CoordToTime(x, clientRect);
			if( _stressDetector.IsVoiced(time) )
			{
				FxReal frequency = _stressDetector.GetFundamentalFrequency(time);
				dc.DrawPoint(x, clientRect.GetHeight() - ((frequency/maxFrequency) * clientRect.GetHeight()));
			}
		}

		FxReal avgFrequency = _stressDetector.GetAvgFrequency();
		dc.SetPen(wxPen(wxColour(0,0,255), 1, wxSOLID));
		dc.DrawLine(wxPoint(clientRect.GetLeft(), clientRect.GetHeight() - ((avgFrequency/maxFrequency) * clientRect.GetHeight())),
					wxPoint(clientRect.GetRight(), clientRect.GetHeight() - ((avgFrequency/maxFrequency) * clientRect.GetHeight())));
	}
}

void FxAudioWidget::_drawEmphasisPlot( wxDC& FxUnused(dc) )
{
//	if( _pAudio && _readyToDraw )
//	{
//		FxReal maxEmphasis = _stressDetector.GetMaxEmphasis();
//		FxReal minEmphasis = _stressDetector.GetMinEmphasis();
//		wxRect clientRect = GetClientRect();
//		for( FxInt32 x = 0; x < clientRect.GetWidth()-1; ++x )
//		{
//			FxReal time = FxTimeManager::CoordToTime(x, clientRect);
//			FxReal nextTime = FxTimeManager::CoordToTime(x+1, clientRect);
//			
//			if( _stressDetector.IsVoiced(time) && _stressDetector.IsVoiced(nextTime) )
//			{
//				FxReal emphasis = _stressDetector.GetEmphasis(time);
//				FxReal nextEmphasis = _stressDetector.GetEmphasis(nextTime);
//				wxPoint thisPoint(x, clientRect.GetHeight() - (((emphasis-minEmphasis)/(maxEmphasis-minEmphasis)) * clientRect.GetHeight()));
//				wxPoint nextPoint(x+1, clientRect.GetHeight() - (((nextEmphasis-minEmphasis)/(maxEmphasis-minEmphasis)) * clientRect.GetHeight()));
//				dc.SetPen(wxPen(wxColour(255,0,255), 1, wxSOLID));
//				dc.DrawLine(thisPoint, nextPoint);
//			}
//		}
//	}
}

void FxAudioWidget::_drawOverlay( wxDC& dc )
{
	if( _pAudio && _readyToDraw )
	{
		wxRect waveformOverlayRect(wxPoint(GetClientRect().GetWidth()-155, 20), wxSize(150,100));
		wxRect acrOverlayRect(wxPoint(GetClientRect().GetWidth()-155, 125), wxSize(150,100));
		//wxRect cmndOverlayRect(wxPoint(GetClientRect().GetWidth()-155, 230), wxSize(150,100));
		dc.SetPen(*wxBLACK_PEN);
		dc.SetBrush(*wxWHITE_BRUSH);
		dc.DrawRectangle(waveformOverlayRect);
		dc.DrawRectangle(acrOverlayRect);
		//dc.DrawRectangle(cmndOverlayRect);
		dc.SetTextForeground(*wxBLACK);
		dc.SetFont( wxFont(8.f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode")) );

		FxReal pixelTime = FxTimeManager::CoordToTime( _lastMousePos.x, GetClientRect() );
		// Only if the time of this pixel column is in the audio range do 
		// we have valid data to draw.
		if( pixelTime >= 0.0f && 
			pixelTime <= static_cast<FxReal>(_pAudio->GetDuration()) - 0.05f )
		{
			// The sample corresponding to the time of this pixel column.
			FxUInt32 sample = _pAudio->GetSampleRate() * pixelTime;
			const FxDReal k8BitRescale  = 1.0 / static_cast<FxDReal>(1 << 8);
			const FxDReal k16BitRescale = 1.0 / static_cast<FxDReal>(1 << 16);
			FxBool is8Bit = _pAudio->GetBitsPerSample() == 8;
			FxUInt32 numSamples = 0.025 * _pAudio->GetSampleRate();

			// Draw the waveform plot
			if( sample + numSamples < _pAudio->GetNumSamples() )
			{
				wxPoint* points = new wxPoint[numSamples];
				for( FxUInt32 sampleIndex = sample; sampleIndex < sample + numSamples; ++sampleIndex )
				{
					FxInt32 normSampleIndex = sampleIndex - sample;
					FxDReal sampleValue = _pAudio->GetSample(sampleIndex) * (is8Bit ? k8BitRescale : k16BitRescale) + 0.5;
					FxInt32 x = (static_cast<FxReal>(normSampleIndex) / static_cast<FxReal>(numSamples)) * waveformOverlayRect.GetWidth() + waveformOverlayRect.GetLeft();
					FxInt32 y = waveformOverlayRect.GetHeight() - (sampleValue * waveformOverlayRect.GetHeight()) + waveformOverlayRect.GetTop();
					points[normSampleIndex] = wxPoint(x,y);
				}
				dc.DrawLines(numSamples, points);

				// Compute the difference function
				FxArray<FxReal> differenceFunc;
				differenceFunc.Reserve(numSamples);
				for( FxUInt32 rho = 0; rho < numSamples; ++rho )
				{
					FxReal differenceSum = 0.0f;
					for( FxUInt32 j = 1; j <= numSamples; ++j )
					{
						FxReal difference = _pAudio->GetSample(sample+j) - 
											_pAudio->GetSample(sample+rho+j);
						difference *= difference;
						differenceSum += difference;
					}
					differenceFunc.PushBack(differenceSum);
				}

				// Compute the cumulative mean normalized difference function
				FxArray<FxReal> cmndFunc;
				cmndFunc.Reserve(numSamples);
				FxReal minCmnd = 0.0f;
				FxReal maxCmnd = 3.0f;
				for( FxUInt32 rho = 0; rho < numSamples; ++rho )
				{
					if( rho == 0 )
					{
						cmndFunc.PushBack(1.0f);
					}
					else
					{
						FxReal mean = 0.0f;
						for( FxUInt32 j = 0; j < rho; ++j )
						{
							mean += differenceFunc[j];
						}
						mean *= (1.0f/static_cast<FxReal>(rho));
						if( mean != 0.0f )
						{
							cmndFunc.PushBack(differenceFunc[rho] / mean);
						}
						else
						{
							cmndFunc.PushBack(1.0f);
						}
					}
				}

				// Find the period of the cumulative mean normalized difference function.
				const FxReal threshold = 0.1f;
				FxReal globalMin = FX_REAL_MAX;
				FxSize globalMinIndex = 0;
				FxSize period = 0;
				for( FxUInt32 i = 0; i < numSamples; ++i )
				{
					if( cmndFunc[i] < threshold )
					{
						period = i;
						break;
					}
					if( cmndFunc[i] < globalMin )
					{
						globalMin = cmndFunc[i];
						globalMinIndex = i;
					}
				}
				if( period == 0 )
				{
					period = globalMinIndex;
				}

				// Draw the cumulative mean normalized difference function
				FxInt32 thresholdLine = acrOverlayRect.GetHeight() - (((threshold-minCmnd)/(maxCmnd-minCmnd)) * acrOverlayRect.GetHeight()) + acrOverlayRect.GetTop();
				dc.SetPen(*wxRED_PEN);
				dc.DrawLine(wxPoint(acrOverlayRect.GetLeft(), thresholdLine), wxPoint(acrOverlayRect.GetRight(), thresholdLine));
				dc.SetPen(*wxBLACK_PEN);
				for( FxUInt32 i = 0; i < numSamples; ++i )
				{
					FxInt32 x = (static_cast<FxReal>(i) / static_cast<FxReal>(numSamples)) * acrOverlayRect.GetWidth() + acrOverlayRect.GetLeft();
					FxInt32 y = acrOverlayRect.GetHeight() - (((cmndFunc[i]-minCmnd)/(maxCmnd-minCmnd)) * acrOverlayRect.GetHeight()) + acrOverlayRect.GetTop();
					points[i] = wxPoint(x,y);
				}
				dc.DrawLines(numSamples, points);
				dc.DrawText(wxString::Format(wxT("frequency: %fHz"), (1.0f/static_cast<FxReal>(period))*_pAudio->GetSampleRate()), acrOverlayRect.GetLeft() + 5, acrOverlayRect.GetTop() + 5);
				delete[] points;

				FxInt32 psdIndex = static_cast<FxInt32>(_frameStep * 
					(static_cast<FxDReal>(GetClientRect().GetHeight() - _lastMousePos.y) / static_cast<FxDReal>(GetClientRect().GetHeight())));
				dc.DrawText(wxString::Format(wxT("mousefq: %fHz"), FxFFT::GetFrequency(psdIndex, _pAudio->GetSampleRate(), _windowLength)), acrOverlayRect.GetLeft() + 5, acrOverlayRect.GetTop() + 17);
			}

			/* Frequency Response Drawing
			// Convert it to an index into the frames of the power spectral
			// density.
			FxInt32 frameIndex = sample / static_cast<FxDReal>(_frameStep);
			FxSize numPsd = _powerSpectralDensity[frameIndex].Length();
			wxPoint* points = new wxPoint[numPsd];
			for( FxSize i = 0; i < numPsd; ++i )
			{
				FxDReal psdValue = _powerSpectralDensity[frameIndex][i];
				FxInt32 x = (static_cast<FxReal>(i) / static_cast<FxReal>(numPsd)) * overlayRect.GetWidth() + overlayRect.GetLeft();
				FxInt32 y = overlayRect.GetTop() + overlayRect.GetHeight() - ((psdValue / _maxPowerSpectralDensityValue) * overlayRect.GetHeight());
				points[i] = wxPoint(x,y);
			}
			dc.DrawLines(numPsd, points);
			delete[] points;
			*/
		}
	}
}

// Draws the phoneme information plot.
void FxAudioWidget::_drawVowelInfoPlot( wxDC& dc )
{
	if( _readyToDraw && _pAudio )
	{
		wxFont smallFont(8, wxSWISS, wxNORMAL, wxNORMAL);
		dc.SetFont(smallFont);
		wxRect clientRect = GetClientRect();
		FxReal maxPower = _stressDetector.GetMaxPower();
		FxReal minPower = _stressDetector.GetMinPower();
		FxReal maxFrequency = _pAudio->GetSampleRate() / 2;

		for( FxSize i = 0; i < _stressDetector.GetNumStressInfos(); ++i )
		{
			const FxStressInformation stressInfo = _stressDetector.GetStressInfo(i);
			if( stressInfo.startTime <= _maxTime && 
				stressInfo.endTime   >= _minTime )
			{
				if( stressInfo.normalizedAvgPower > _gestureConfig.sdStressThreshold )
				{
					dc.SetPen(*wxRED_PEN);
				}
				else
				{
					dc.SetPen(*wxWHITE_PEN);
				}
				FxInt32 startX = FxTimeManager::TimeToCoord(stressInfo.startTime, clientRect);
				FxInt32 endX = FxTimeManager::TimeToCoord(stressInfo.endTime, clientRect);
				dc.DrawLine(startX, clientRect.GetTop(), startX, clientRect.GetBottom());
				dc.DrawLine(endX, clientRect.GetTop(), endX, clientRect.GetBottom());

				FxInt32 avgPowerY = clientRect.GetHeight() - (((stressInfo.averagePower-minPower)/(maxPower-minPower)) * clientRect.GetHeight());
				FxInt32 minPowerY = clientRect.GetHeight() - (((stressInfo.minPower-minPower)/(maxPower-minPower)) * clientRect.GetHeight());
				FxInt32 maxPowerY = clientRect.GetHeight() - (((stressInfo.maxPower-minPower)/(maxPower-minPower)) * clientRect.GetHeight());
				dc.SetPen(wxPen(wxColour(255,0,255), 1, wxSOLID));
				dc.DrawLine(startX, avgPowerY, endX, avgPowerY);
				dc.SetPen(wxPen(wxColour(0,0,255), 1, wxSOLID));
				dc.DrawLine(startX, minPowerY, endX, minPowerY);
				dc.SetPen(wxPen(wxColour(255,0,0), 1, wxSOLID));
				dc.DrawLine(startX, maxPowerY, endX, maxPowerY);

				FxInt32 freqStartY = clientRect.GetHeight() - ((stressInfo.startPitch/maxFrequency) * clientRect.GetHeight());
				FxInt32 freqMidY   = clientRect.GetHeight() - ((stressInfo.midPitch/maxFrequency) * clientRect.GetHeight());
				FxInt32 freqEndY   = clientRect.GetHeight() - ((stressInfo.endPitch/maxFrequency) * clientRect.GetHeight());
				dc.SetPen(wxPen(wxColour(255,128,0), 1, wxSOLID));
				dc.DrawLine(startX, freqStartY, (startX + endX) / 2, freqMidY);
				dc.DrawLine((startX+endX)/2, freqMidY, endX, freqEndY);

				wxString relativePitchLabel;
				switch( stressInfo.relativePitch )
				{
				case IRP_Unknown:
					relativePitchLabel = wxT("Unknown");
					break;
				case IRP_High:
					relativePitchLabel = wxT("High");
					break;
				case IRP_Neutral:
					relativePitchLabel = wxT("Neutral");
					break;
				case IRP_Low:
					relativePitchLabel = wxT("Low");
					break;
				}
				wxString contourLabel;
				switch( stressInfo.contour )
				{
				default:
				case IC_Unknown:
					contourLabel = wxT("Unknown");
					break;
				case IC_Flat:
					contourLabel = wxT("Flat");
					break;
				case IC_Rising:
					contourLabel = wxT("Rising");
					break;
				case IC_Falling:
					contourLabel = wxT("Falling");
					break;
				case IC_Hump:
					contourLabel = wxT("Hump");
					break;
				case IC_Dip:
					contourLabel = wxT("Dip");
					break;
				}
				wxString categoryLabel;
				switch( stressInfo.stressCategory )
				{
				default:
				case SC_Unknown:
					categoryLabel = wxT("Unknown");
					break;
				case SC_Initial:
					categoryLabel = wxT("Initial");
					break;
				case SC_Quick:
					categoryLabel = wxT("Quick");
					break;
				case SC_Normal:
					categoryLabel = wxT("Normal");
					break;
				case SC_Isolated:
					categoryLabel = wxT("Isolated");
					break;
				case SC_Final:
					categoryLabel = wxT("Final");
					break;
				}
				dc.DrawText(relativePitchLabel, startX + 1, clientRect.GetTop() + 15);
				dc.DrawText(contourLabel, startX + 1, clientRect.GetTop() + 26);
				dc.DrawText(wxString::Format(wxT("%0.2f"), stressInfo.normalizedAvgPower), startX + 1, clientRect.GetTop() + 37);
				if( stressInfo.normalizedAvgPower > _gestureConfig.sdStressThreshold )
				{
					dc.DrawText(wxString::Format(wxT("%0.2f"), stressInfo.localTimeScale), startX + 1, clientRect.GetTop() + 48);
					dc.DrawText(categoryLabel, startX + 1, clientRect.GetTop() + 59);
				}
			}
		}
	}
}

void FxAudioWidget::OnResetView( wxCommandEvent& event )
{
	if( _pAudio )
	{
		FxTimeManager::RequestTimeChange( FxTimeManager::GetMinimumTime(),
			FxTimeManager::GetMaximumTime() );
	}
	event.Skip();
}

void FxAudioWidget::OnToggleFxAudioPlotType( wxCommandEvent& event )
{
	if( _plotType == APT_Spectral )
	{
		_plotType = APT_Waveform;
	}
	else if( _plotType == APT_Waveform )
	{
		_plotType = APT_Spectral;
	}
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidget::OnSetSpectralPlotType( wxCommandEvent& event )
{
	SetPlotType(APT_Spectral);
	event.Skip();
}

void FxAudioWidget::OnSetWaveformPlotType( wxCommandEvent& event )
{
	SetPlotType(APT_Waveform);
	event.Skip();
}

void FxAudioWidget::OnEditOptions( wxCommandEvent& event )
{
	ShowOptionsDialog();
	event.Skip();
}

void FxAudioWidget::OnPaint( wxPaintEvent& FxUnused(event) )
{
	wxStopWatch watch;
	wxPaintDC front( this );

	if( FxTrue == _readyToDraw )
	{
		if( _pAudio )
		{
			wxFont smallFont(8.f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode"));
			FxInt32 doubleSmallFontPointSize = smallFont.GetPointSize() * 2;
			wxString longText = _plotType == APT_Spectral ? wxT("0000") : wxT("000");
			FxInt32 textLength, trash;
			wxMemoryDC plotDC;
			plotDC.GetTextExtent(longText, &textLength, &trash);
			plotDC.SetFont(smallFont);
			plotDC.SetTextForeground(wxColour(_textColor.GetRedByte(),
				                              _textColor.GetGreenByte(),
				                              _textColor.GetBlueByte()));
			plotDC.Clear();

			wxRect clientRect = GetClientRect();
			FxInt32 width     = clientRect.GetWidth();
			FxInt32 height    = clientRect.GetHeight();

			wxBitmap plotBitmap(width, height);
			
			FxBool showStress = static_cast<FxBool>(*aw_showstress);

			FxTimeManager::GetGridInfos(_timeGridInfos, clientRect);

			switch( _plotType )
			{
				// Spectral is default.
			default:
			case APT_Spectral:
				{
					wxImage plotImage = plotBitmap.ConvertToImage();
					// Draw the spectral plot.
					_drawSpectralPlot(plotImage);

					if( showStress )
					{
						plotBitmap = wxBitmap(plotImage);
						wxMemoryDC stressDC;
						stressDC.Clear();
						stressDC.SetFont(smallFont);
						stressDC.SetTextForeground(wxColour(_textColor.GetRedByte(),
							                                _textColor.GetGreenByte(),
							                                _textColor.GetBlueByte()));
						stressDC.SelectObject(plotBitmap);
//						_drawPowerPlot(stressDC);
//						_drawFrequencyPlot(stressDC);
//						_drawEmphasisPlot(stressDC);
//						_drawOverlay(stressDC);
						_drawVowelInfoPlot(stressDC);
						stressDC.SelectObject(wxNullBitmap);
    					plotImage = plotBitmap.ConvertToImage();
					}

					// Draw the shaded axes.
					_drawShadedAxes(plotImage, doubleSmallFontPointSize, textLength);
					// Select the image with the shaded axes into the dc.
					plotBitmap = wxBitmap(plotImage);
					plotDC.SelectObject(plotBitmap);
					// Draw the spectral axis labels.
					_drawSpectralAxisLabels(plotDC, doubleSmallFontPointSize);
				}
				break;
			case APT_Waveform:
				{
					wxMemoryDC waveDC;
					waveDC.Clear();
					waveDC.SetFont(smallFont);
					waveDC.SetTextForeground(wxColour(_textColor.GetRedByte(),
						                              _textColor.GetGreenByte(),
						                              _textColor.GetBlueByte()));
					waveDC.SelectObject(plotBitmap);
					// Draw the waveform background grid.
					_drawWaveformBackgroundGrid(waveDC);
					// Draw the waveform plot.
					_drawWaveformPlot(waveDC);

					if( showStress )
					{
//						_drawPowerPlot(waveDC);
//						_drawFrequencyPlot(waveDC);
//						_drawEmphasisPlot(waveDC);
//						_drawOverlay(waveDC);
						_drawVowelInfoPlot(waveDC);
					}

					waveDC.SelectObject(wxNullBitmap);
					wxImage plotImage = plotBitmap.ConvertToImage();
					// Draw the shaded axes.
					_drawShadedAxes(plotImage, doubleSmallFontPointSize, textLength);
					// Select the image with the shaded axes into the dc.
					plotBitmap = wxBitmap(plotImage);
					plotDC.SelectObject(plotBitmap);
					// Draw the waveform axis labels.
					_drawWaveformAxisLabels(plotDC, doubleSmallFontPointSize);
				}
				break;
			}

			if( _hasFocus )
			{
				plotDC.SetBrush(*wxTRANSPARENT_BRUSH);
				plotDC.SetPen(wxPen(FxStudioOptions::GetFocusedWidgetColour(), 1, wxSOLID));
				plotDC.DrawRectangle(GetClientRect());
			}

			// Swap to the front buffer.
			front.Blit(front.GetLogicalOrigin(), front.GetSize(), &plotDC, plotDC.GetLogicalOrigin(), wxCOPY);
			plotDC.SelectObject(wxNullBitmap);
		}
		else
		{
			wxBrush backgroundBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE), wxSOLID);
			front.SetBackground(backgroundBrush);
			front.Clear();
		}
	}
	else
	{
		wxBrush backgroundBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE), wxSOLID);
		front.SetBackground(backgroundBrush);
		front.Clear();
	}
	lastPaintTime = watch.Time();
}

void FxAudioWidget::_drawShadedAxes( wxImage& plotImage, FxInt32 doubleSmallFontPointSize, FxInt32 textLength )
{
	// Prepare to draw the shaded axes.
	wxRect clientRect = GetClientRect();
	FxInt32 width     = clientRect.GetWidth();
	FxInt32 height    = clientRect.GetHeight();

	FxByte* pixels = plotImage.GetData();

	// Draw the shaded axes by determining if the pixel is in an axis "shade"
	// and modifying the color value of the pixel accordingly.  Multiply by 2 
	// so that the vertical axis extends out enough to handle numbers up to 
	// 99999.
	for( FxInt32 w = 0; w < width; ++w )
	{
		for( FxInt32 h = 0; h < height; ++h )
		{
			FxInt32 pixelIndex = ((height - 1 - h) * width + w) * 3;
			if( _timeAxisOnTop == FxTrue )
			{
				if( (h > height - doubleSmallFontPointSize) || 
					(w < textLength							&& 
					h <= height - doubleSmallFontPointSize) )
				{
					pixels[pixelIndex + 0] = pixels[pixelIndex + 0] / 2;
					pixels[pixelIndex + 1] = pixels[pixelIndex + 1] / 2;
					pixels[pixelIndex + 2] = pixels[pixelIndex + 2] / 2;
				}
			}
			else 
			{
				if( (h < doubleSmallFontPointSize) || 
					(w < textLength				   && 
					h >= doubleSmallFontPointSize) )
				{
					pixels[pixelIndex + 0] = pixels[pixelIndex + 0] / 2;
					pixels[pixelIndex + 1] = pixels[pixelIndex + 1] / 2;
					pixels[pixelIndex + 2] = pixels[pixelIndex + 2] / 2;
				}
			}
		}
	}
}

void FxAudioWidget::_drawSpectralPlot( wxImage& plotImage )
{
	if( _pAudio )
	{
		wxRect clientRect = GetClientRect();
		FxInt32 width     = clientRect.GetWidth();
		FxInt32 height    = clientRect.GetHeight();

		FxDReal audioSampleRate    = _pAudio->GetSampleRate();
		FxDReal audioDuration      = _pAudio->GetDuration();

		FxByte* spectralPlotPixels = plotImage.GetData();
		// Initialize the spectral plot by filling it with the low energy color.
		for( FxInt32 i = 0; i < width * height; ++i )
		{
			FxInt32 pixelIndex = i * 3;
			spectralPlotPixels[pixelIndex + 0] = _lowEnergyColor.GetRedByte();
			spectralPlotPixels[pixelIndex + 1] = _lowEnergyColor.GetGreenByte();
			spectralPlotPixels[pixelIndex + 2] = _lowEnergyColor.GetBlueByte();
		}
		// Draw spectral plot.
		FxInt32 numFrames = static_cast<FxInt32>(_powerSpectralDensity.Length());
		// Draw each column of the plot.
		for( FxInt32 w = 0; w < width; ++w )
		{
			FxReal  pixelTime = FxTimeManager::CoordToTime( w, clientRect );
			// Only if the time of this pixel column is in the audio range do 
			// we have valid data to draw.
			if( pixelTime >= 0.0f && 
				pixelTime <= static_cast<FxReal>(audioDuration) )
			{
				// The sample corresponding to the time of this pixel column.
				FxDReal sample = audioSampleRate * pixelTime;
				// Convert it to an index into the frames of the power spectral
				// density.
				FxInt32 frameIndex = sample / static_cast<FxDReal>(_frameStep);
				// Clamp the index and the index of the frame to the immediate
				// right of the current frame (we form unit squares between 
				// adjacent power spectral density values and interpolate the
				// value based on a weighted average of the distance of this
				// pixel to each of the four corners of the unit square).
				FxInt32 frameIndexPlusOne = frameIndex + 1;
				if( frameIndex >= numFrames )
				{
					frameIndex = numFrames - 1;
					frameIndexPlusOne = frameIndex;
				}
				if( frameIndexPlusOne >= numFrames )
				{
					frameIndexPlusOne = frameIndex;
				}
				// The parametric distance of this pixel to the frame to
				// the immediate right.
				FxDReal framePct = sample / 
					static_cast<FxDReal>(_frameStep) - frameIndex;
				// Draw each pixel in this column.
				for( FxInt32 h = 0; h < height; ++h )
				{
					// The power spectral density value corresponding to this
					// height in the pixel column.
					FxInt32 psdIndex = static_cast<FxInt32>(_frameStep * 
						(static_cast<FxDReal>(h) / static_cast<FxDReal>(height)));
					FxInt32 psdIndexPlusOne = psdIndex + 1;
					// Same type of clamping that goes on with the frameIndex
					// above.
					if( psdIndex >= _frameStep )
					{
						psdIndex = _frameStep - 1;
						psdIndexPlusOne = psdIndex;
					}
					if( psdIndexPlusOne >= _frameStep )
					{
						psdIndexPlusOne = psdIndex;
					}
					// The parametric distance of this pixel to the power 
					// spectral density value "above" this one.
					FxDReal psdPct = _frameStep * 
						(static_cast<FxDReal>(h) / 
						static_cast<FxDReal>(height)) - psdIndex;
					FxDReal oneMinusFramePct = 1.0 - framePct;
					FxDReal oneMinusPsdPct = 1.0 - psdPct;

					// Compute the weights of the corners of the square.
					FxDReal blw = oneMinusFramePct * oneMinusPsdPct;
					FxDReal tlw = oneMinusFramePct * psdPct;
					FxDReal trw = framePct * psdPct;
					FxDReal brw = framePct * oneMinusPsdPct;			

					// Compute the weighted value of the corners of the square.
					FxDReal blv = blw * 
						_powerSpectralDensity[frameIndex][psdIndex];
					FxDReal tlv = tlw * 
						_powerSpectralDensity[frameIndex][psdIndexPlusOne];
					FxDReal trv = trw * 
						_powerSpectralDensity[frameIndexPlusOne][psdIndexPlusOne];
					FxDReal brv = brw * 
						_powerSpectralDensity[frameIndexPlusOne][psdIndex];

					// Add them up.
					FxDReal value = blv + tlv + trv + brv;
					// Look up the color for this value in the color table.
					FxColorTableEntry color = _colorLookup(value);
					// Directly modify this pixel's bytes in the image and
					// invert the vertical axis while doing so. 
					FxInt32 pixelIndex = ((height - 1 - h) * width + w) * 3;
					spectralPlotPixels[pixelIndex + 0] = color.red;
					spectralPlotPixels[pixelIndex + 1] = color.green;
					spectralPlotPixels[pixelIndex + 2] = color.blue;
				}
			}
		}
	}
}

void FxAudioWidget::_drawSpectralAxisLabels( wxMemoryDC& dc, FxInt32 doubleSmallFontPointSize )
{
	if( _pAudio )
	{
		wxRect clientRect = GetClientRect();
		FxInt32 height    = clientRect.GetHeight();

		FxDReal audioSampleRate = _pAudio->GetSampleRate();

		dc.SetFont( wxFont(8.f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode")) );

		// Draw the axis systems.
		dc.SetPen(wxPen(wxColour(_textColor.GetRedByte(),
			_textColor.GetGreenByte(),
			_textColor.GetBlueByte() ), 
			1, wxSOLID ) );
		// Vertical axis goes from 0 Hz to the Nyquist Frequency.
		FxInt32 nyquistFreqency = audioSampleRate / 2;
		// Maximum resolution is 100 Hz.
		FxInt32 resolution = 100;
		// Find the number of ticks required for maximum resolution.
		FxInt32 numTicks = nyquistFreqency / resolution;
		FxInt32 xExtent, yExtent;
		// All numbers should be the same height.
		dc.GetTextExtent( _("0"), &xExtent, &yExtent );
		// Add an extra numTicks to make sure there is a gap between
		// labels.
		while( height < (numTicks * yExtent) + numTicks )
		{
			// Need to scale resolution.
			resolution *= 2;
			numTicks = nyquistFreqency / resolution;
		}
		FxReal tickHeight = static_cast<FxReal>(height / static_cast<FxReal>(numTicks));
		// Start at 0 Hz and work up to the Nyquist Frequency, 
		// increasing by resolution Hz at each step.
		FxInt32 frequency = 0;
		for( FxInt32 i = 0; i < numTicks; ++i )
		{
			// Don't label the very first and very last ticks because
			// they're half off-screen.
			if( i != 0 && i != numTicks )
			{
				wxString label = wxString::Format( _("%d"), frequency );
				dc.GetTextExtent( label, &xExtent, &yExtent );

				dc.SetTextForeground(wxColour(0, 0, 0));
				dc.DrawText( label, 1, height - (((i+1) * tickHeight) - (yExtent / 2)) + 1 );
				dc.SetTextForeground(wxColour(_textColor.GetRedByte(),
					_textColor.GetGreenByte(),
					_textColor.GetBlueByte() ) );
				dc.DrawText( label, 0, height - (((i+1) * tickHeight) - (yExtent / 2)) );
				dc.DrawLine( xExtent, height - (((i+1) * tickHeight) - yExtent),
					doubleSmallFontPointSize * 2,
					height - (((i+1) * tickHeight) - yExtent) );
			}
			frequency += resolution;
		}
		// Draw the horizontal (time) axis.
		for( FxSize i = 0; i < _timeGridInfos.Length(); ++i )
		{
			FxInt32 x = FxTimeManager::TimeToCoord( _timeGridInfos[i].time, clientRect );
			dc.GetTextExtent( _timeGridInfos[i].label, &xExtent, &yExtent );
			FxInt32 xPos = x - (xExtent / 2);
			if( _timeAxisOnTop == FxTrue )
			{
				if( xPos > doubleSmallFontPointSize * 2 )
				{
					dc.SetTextForeground(wxColour(0, 0, 0));
					dc.DrawText( _timeGridInfos[i].label, xPos+1, 1 );
					dc.SetTextForeground(wxColour(_textColor.GetRedByte(),
						_textColor.GetGreenByte(),
						_textColor.GetBlueByte() ) );
					dc.DrawText( _timeGridInfos[i].label, xPos, 0 );
					dc.DrawLine( x, yExtent, x, doubleSmallFontPointSize );
				}
			}
			else
			{
				if( xPos > doubleSmallFontPointSize * 2 )
				{
					dc.SetTextForeground(wxColour(0, 0, 0));
					dc.DrawText( _timeGridInfos[i].label, xPos+1, height - 11 );
					dc.SetTextForeground(wxColour(_textColor.GetRedByte(),
						_textColor.GetGreenByte(),
						_textColor.GetBlueByte() ) );
					dc.DrawText( _timeGridInfos[i].label, xPos, height - 12 );
					dc.DrawLine( x, height - yExtent + 3, x, height - doubleSmallFontPointSize );
				}
			}
		}
	}
}

void FxAudioWidget::_drawWaveformBackgroundGrid( wxMemoryDC& dc )
{
	wxRect clientRect = GetClientRect();
	FxInt32 width     = clientRect.GetWidth();
	FxInt32 height    = clientRect.GetHeight();

	dc.SetBackground(wxBrush(wxColour(_waveformBackgroundColor.GetRedByte(),
		_waveformBackgroundColor.GetGreenByte(),
		_waveformBackgroundColor.GetBlueByte() ),
		wxSOLID ) );
	dc.Clear();

	// Draw the vertical axis system.
	// Draw the amplitude percentage lines.
	dc.SetPen(wxPen(wxColour(_waveformGridColor.GetRedByte(),
		_waveformGridColor.GetGreenByte(),
		_waveformGridColor.GetBlueByte() ), 
		1, wxSOLID ) );
	FxReal tickHeight = static_cast<FxReal>(height / 40.0f);

	//@note This code to determine the proper position for drawing horizontal 
	//      grid lines is duplicated from _drawWaveformAxisLabels().
	// Draw the horizontal lines for the vertical axis (amplitude percentage).
	FxInt32 amplitudePercentage = 95;
	// Only using yExtent with numerical strings here, so any text 
	// should be the the same height.
	FxInt32 xExtent, yExtent;
	dc.GetTextExtent(_("0"), &xExtent, &yExtent);
	// Add the extra 38 to make sure there is a gap between labels
	// so they don't overlap.
	FxInt32 heightRequiredToDrawAllLabels = (38 * yExtent) + 38;
	for( FxInt32 i = 0; i < 39; ++i )
	{
		// Only draw labels that can fit.
		// Always draw zero (when i is 19).
		if( height >= heightRequiredToDrawAllLabels || 
			i == 19 ||
			((height >= (heightRequiredToDrawAllLabels / 2) &&
			height < heightRequiredToDrawAllLabels) && 
			i%2 == 0) ||
			((height >= (heightRequiredToDrawAllLabels / 4) &&
			height < (heightRequiredToDrawAllLabels / 2)) &&
			((i < 20) ? i%4 == 0 : (i-2)%4 == 0)) ||
			((height >= (heightRequiredToDrawAllLabels / 6) &&
			height < (heightRequiredToDrawAllLabels / 4)) &&
			(i == 0 || i == 38 || i == 9 || i == 29)) ||
			((height >= (heightRequiredToDrawAllLabels / 8) &&
			height < (heightRequiredToDrawAllLabels / 6)) &&
			(i == 0 || i == 38)) )
		{
			dc.DrawLine(0, (i+1) * tickHeight, width, (i+1) * tickHeight);
		}
		amplitudePercentage -= 5;
	}

	// Draw the horizontal (time) axis.
	for( FxSize i = 0; i < _timeGridInfos.Length(); ++i )
	{
		FxInt32 x = FxTimeManager::TimeToCoord( _timeGridInfos[i].time, clientRect );
		dc.DrawLine( x, 0, x, height );
	}

	// Draw the zero line.
	dc.SetPen(wxPen(wxColour(_waveformZeroAxisColor.GetRedByte(),
		_waveformZeroAxisColor.GetGreenByte(),
		_waveformZeroAxisColor.GetBlueByte() ), 
		1, wxSOLID ) );
	dc.DrawLine( 0, height * 0.5f, width, height * 0.5f );
	// Draw the 90% threshold lines.
	dc.SetPen(wxPen(wxColour(_waveformThresholdLineColor.GetRedByte(),
		_waveformThresholdLineColor.GetGreenByte(),
		_waveformThresholdLineColor.GetBlueByte() ),
		1, wxSOLID ) );
	dc.DrawLine( 0, 2 * tickHeight, width, 2 * tickHeight );
	dc.DrawLine( 0, 38 * tickHeight, width, 38 * tickHeight );
}

void FxAudioWidget::_drawWaveformPlot( wxMemoryDC& dc )
{
	if( _pAudio )
	{
		wxRect clientRect = GetClientRect();
		FxInt32 width     = clientRect.GetWidth();
		FxInt32 height    = clientRect.GetHeight();

		FxInt32 audioSampleCount   = _pAudio->GetNumSamples();
		FxDReal audioSampleRate    = _pAudio->GetSampleRate();
		FxDReal invAudioSampleRate = 1.0 / audioSampleRate;
		FxDReal audioDuration      = _pAudio->GetDuration();

		// If _minTime is less than zero and / or _maxTime is greater
		// than audio duration, we can't start the drawing on pixel
		// column zero and end on pixel column width.  Some other
		// start and end points need to be determined based on
		// _minTime and _maxTime, and the loop needs to go from
		// start pixel column to end pixel column.
		FxInt32 startPixel = 0;
		FxInt32 endPixel = 0;
		FxReal minSampleTime = 0.0f;
		FxReal maxSampleTime = 0.0f;
		if( _minTime < 0.0f )
		{
			startPixel = FxTimeManager::TimeToCoord( 0.0f, clientRect );
			minSampleTime = 0.0f;
		}
		else
		{
			if( _minTime < audioDuration )
			{
				startPixel = 0;
				minSampleTime = _minTime;
			}
		}
		if( _maxTime > audioDuration )
		{
			endPixel = FxTimeManager::TimeToCoord( audioDuration, clientRect );
			maxSampleTime = audioDuration;
		}
		else
		{
			if( _maxTime > 0.0f )
			{
				endPixel = width;
				maxSampleTime = _maxTime;
			}
		}
		// Trap the case where both _minTime and _maxTime are greater
		// than audioDuration (the waveform has been scrolled to the
		// left off-screen) and set the variables up such that nothing
		// is drawn.  endPixel is set to one and not zero to avoid
		// a divide by zero condition when calculating the resolution.
		if( endPixel < startPixel )
		{
			startPixel    = 0;
			endPixel      = 1;
			minSampleTime = 0.0f;
			maxSampleTime = 0.0f;
		}

		// Figure out the minimum and maximum samples that should be
		// displayed for the current time range.
		FxInt32 minSampleDisplayed = minSampleTime * audioSampleRate;
		FxInt32 maxSampleDisplayed = maxSampleTime * audioSampleRate;

		FxDReal resolution = 
			static_cast<FxDReal>(maxSampleDisplayed - minSampleDisplayed)
			/ static_cast<FxDReal>(endPixel - startPixel);
		const FxDReal k8BitRescale  = 1.0 / static_cast<FxDReal>(1 << 8);
		const FxDReal k16BitRescale = 1.0 / static_cast<FxDReal>(1 << 16);
		FxBool is8Bit = _pAudio->GetBitsPerSample() == 8;

		wxPen wavformPlotPen = wxPen(wxColour(_waveformPlotColor.GetRedByte(),
			_waveformPlotColor.GetGreenByte(),
			_waveformPlotColor.GetBlueByte()), 
			1, wxSOLID);

		dc.SetPen( wavformPlotPen );

		// Draw the waveform plot.
		// 100 samples per pixel is a nice cutoff point for switching
		// between the approximated drawing and the exact drawing.
		if( resolution > 100.0 )
		{
			// Use fast vertical line approximation.
			FxDReal minSample  = 0.0;
			FxDReal maxSample  = 0.0;
			FxDReal currSample = 0.0;
			FxInt32 res = static_cast<FxInt32>(resolution);
			for( FxInt32 w = startPixel; w < endPixel; ++w )
			{
				minSample = 0.0;
				maxSample = 0.0;
				// Find the max and min values in sample range 
				// represented by each pixel
				FxInt32 offset = 
					static_cast<FxInt32>((w - startPixel) * resolution);
				for( FxInt32 i = 0; i < res; ++i )
				{
					FxInt32 sampleIndex = minSampleDisplayed + offset + i;
					currSample = _pAudio->GetSample(sampleIndex);
					minSample = FxMin<FxDReal>(currSample, minSample);
					maxSample = FxMax<FxDReal>(currSample, maxSample);
				}

				// Transform the sample to [0,1].
				if( is8Bit )
				{
					minSample = minSample * k8BitRescale + 0.5;
					maxSample = maxSample * k8BitRescale + 0.5;
				}
				else
				{
					minSample = minSample * k16BitRescale + 0.5;
					maxSample = maxSample * k16BitRescale + 0.5;
				}

				dc.DrawLine(w, height - (minSample * height), w, height - (maxSample * height));
			}
		}
		else
		{
			// Use exact sample drawing.
			// Calculate the needed values for the first pixel
			FxInt32 currSample = minSampleDisplayed;
			FxDReal sampleValue = _pAudio->GetSample( currSample );
			FxDReal sampleTime  = static_cast<FxDReal>(currSample) * invAudioSampleRate;

			FxInt32 samplePixel = FxTimeManager::TimeToCoord( sampleTime, clientRect );

			// Transform the sample to [0,1].
			if( is8Bit )
			{
				sampleValue = sampleValue * k8BitRescale + 0.5;
			}
			else
			{
				sampleValue = sampleValue * k16BitRescale + 0.5;
			}

			FxInt32 X = samplePixel;
			FxInt32 Y = height - (sampleValue * height);

			for( ; currSample <= maxSampleDisplayed; ++currSample )
			{
				// We can't draw a line from the very last sample.
				if( currSample != audioSampleCount )
				{
					FxDReal sampleP1Value = _pAudio->GetSample(currSample + 1);
					FxDReal sampleP1Time  = static_cast<FxDReal>(currSample + 1) * invAudioSampleRate;

					FxInt32 sampleP1Pixel = FxTimeManager::TimeToCoord( sampleP1Time, clientRect );

					// Transform the sample to [0,1].
					if( is8Bit )
					{
						sampleP1Value = sampleP1Value * k8BitRescale + 0.5;
					}
					else
					{
						sampleP1Value = sampleP1Value * k16BitRescale + 0.5;
					}

					FxInt32 Xp1 = sampleP1Pixel;
					FxInt32 Yp1 = height - (sampleP1Value * height);

					dc.DrawLine( X, Y, Xp1, Yp1 );

					X = Xp1;
					Y = Yp1;
				}
			}
		}
	}
}

void FxAudioWidget::_drawWaveformAxisLabels( wxMemoryDC& dc, FxInt32 doubleSmallFontPointSize )
{
	wxRect clientRect = GetClientRect();
	FxInt32 height    = clientRect.GetHeight();

	FxReal tickHeight = static_cast<FxReal>(height / 40.0f);

	// Draw the axis system text.
	FxInt32 amplitudePercentage = 95;
	dc.SetFont( wxFont(8.f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode")) );
	// Only using yExtent with numerical strings here, so any text 
	// should be the the same height.
	FxInt32 xExtent, yExtent;
	dc.GetTextExtent( _("0"), &xExtent, &yExtent );
	// Add the extra 38 to make sure there is a gap between labels
	// so they don't overlap.
	FxInt32 heightRequiredToDrawAllLabels = (38 * yExtent) + 38;
	for( FxInt32 i = 0; i < 39; ++i )
	{
		// Only draw labels that can fit.
		// Always draw zero (when i is 19).
		if( height >= heightRequiredToDrawAllLabels || 
			i == 19 ||
			((height >= (heightRequiredToDrawAllLabels / 2) &&
			height < heightRequiredToDrawAllLabels) && 
			i%2 == 0) ||
			((height >= (heightRequiredToDrawAllLabels / 4) &&
			height < (heightRequiredToDrawAllLabels / 2)) &&
			((i < 20) ? i%4 == 0 : (i-2)%4 == 0)) ||
			((height >= (heightRequiredToDrawAllLabels / 6) &&
			height < (heightRequiredToDrawAllLabels / 4)) &&
			(i == 0 || i == 38 || i == 9 || i == 29)) ||
			((height >= (heightRequiredToDrawAllLabels / 8) &&
			height < (heightRequiredToDrawAllLabels / 6)) &&
			(i == 0 || i == 38)) )
		{
			wxString label = wxString::Format( _("%d"), amplitudePercentage );
			FxInt32 labelHeight = ((i+1) * tickHeight) - (yExtent / 2);
			dc.SetTextForeground(wxColour(0, 0, 0));
			dc.DrawText( label, 1, labelHeight+1 );
			dc.SetTextForeground(wxColour(_textColor.GetRedByte(),
				_textColor.GetGreenByte(),
				_textColor.GetBlueByte() ) );
			dc.DrawText( label, 0, labelHeight );
		}
		amplitudePercentage -= 5;
	}

	for( FxSize i = 0; i < _timeGridInfos.Length(); ++i )
	{
		FxInt32 x = FxTimeManager::TimeToCoord(_timeGridInfos[i].time, clientRect);
		dc.GetTextExtent( _timeGridInfos[i].label, &xExtent, &yExtent );
		FxInt32 xPos = x - (xExtent / 2);
		if( _timeAxisOnTop == FxTrue )
		{
			if( xPos > doubleSmallFontPointSize )
			{
				dc.SetTextForeground(wxColour(0, 0, 0));
				dc.DrawText( _timeGridInfos[i].label, xPos+1, 1 );
				dc.SetTextForeground(wxColour(_textColor.GetRedByte(),
					_textColor.GetGreenByte(),
					_textColor.GetBlueByte() ) );
				dc.DrawText( _timeGridInfos[i].label, xPos, 0 );
			}
		}
		else
		{
			if( xPos > doubleSmallFontPointSize )
			{
				dc.SetTextForeground(wxColour(0, 0, 0));
				dc.DrawText( _timeGridInfos[i].label, xPos+1, height - 11 );
				dc.SetTextForeground(wxColour(_textColor.GetRedByte(),
					_textColor.GetGreenByte(),
					_textColor.GetBlueByte() ) );
				dc.DrawText( _timeGridInfos[i].label, xPos, height - 12 );
			}
		}
	}
}

void FxAudioWidget::OnPlayAudio( wxCommandEvent& FxUnused(event) )
{
	FxStudioAnimPlayer* animPlayer = FxStudioAnimPlayer::Instance();
	if( animPlayer )
	{
		// Play the current audio window.
		animPlayer->Play(FxFalse, FxTrue);
	}
}

void FxAudioWidget::OnToggleStress( wxCommandEvent& FxUnused(event) )
{
	static FxBool showStress = FxTrue;
	wxString command = wxString::Format(wxT("set -n \"aw_showstress\" -v \"%d\""), showStress ? 1 : 0);
	FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
	showStress = !showStress;
}

void FxAudioWidget::OnMouseWheel( wxMouseEvent& event )
{
	FxZoomDir zoomDir = ZD_Out;
	if( event.GetWheelRotation() < 0 )
	{
		zoomDir = ZD_In;
	}
	_zoom(event.GetPosition(), _mouseWheelZoomFactor, zoomDir);
	event.Skip();
}

void FxAudioWidget::OnLeftButtonDown( wxMouseEvent& event )
{
	SetFocus();
	if( !HasCapture() )
	{
		CaptureMouse();
	}
	event.Skip();
}

void FxAudioWidget::OnLeftButtonUp( wxMouseEvent& event )
{
	if( HasCapture() )
	{
		ReleaseMouse();
	}
	event.Skip();
}

void FxAudioWidget::OnMouseMove( wxMouseEvent& event )
{
	wxPoint thisPos = event.GetPosition();
	if( (event.LeftIsDown() && event.ControlDown()) || event.MiddleIsDown() )
	{
		wxRect clientRect  = GetClientRect();
		FxReal deltaTime  = FxTimeManager::CoordToTime( thisPos.x, clientRect ) -
			FxTimeManager::CoordToTime( _lastMousePos.x, clientRect );
		FxTimeManager::RequestTimeChange( _minTime - deltaTime, _maxTime - deltaTime );
	}

	_lastMousePos = thisPos;
//	if( static_cast<FxBool>(*aw_showstress) )
//	{
//		wxWindowDC windc(this);
//		_drawOverlay(windc);
//	}

	event.Skip();
}

void FxAudioWidget::OnRightButtonDown( wxMouseEvent& event )
{
	SetFocus();
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidget::OnRightButtonUp( wxMouseEvent& event )
{
	wxMenu popupMenu;
	popupMenu.AppendCheckItem(MenuID_AudioWidgetSpectralPlot, _("Spectral Plot\tCtrl+T"), _("Display the audio as a power spectral density plot"));
	popupMenu.AppendCheckItem(MenuID_AudioWidgetWaveformPlot, _("Waveform Plot\tCtrl+T"), _("Display the audio as a waveform plot"));

	if( _plotType == APT_Spectral )
	{
		popupMenu.Check(MenuID_AudioWidgetSpectralPlot, FxTrue);
		popupMenu.Check(MenuID_AudioWidgetWaveformPlot, FxFalse);
	}
	else if( _plotType == APT_Waveform )
	{
		popupMenu.Check(MenuID_AudioWidgetWaveformPlot, FxTrue);
		popupMenu.Check(MenuID_AudioWidgetSpectralPlot, FxFalse);
	}
	
	popupMenu.SetTitle(_("Audio Plot Type"));
	PopupMenu( &popupMenu, event.GetPosition() );

	event.Skip();
}

void FxAudioWidget::OnMiddleButtonDown( wxMouseEvent& event )
{
	SetFocus();
	Refresh(FALSE);

	if( !HasCapture() )
	{
		CaptureMouse();
	}
	event.Skip();
}

void FxAudioWidget::OnMiddleButtonUp( wxMouseEvent& event )
{
	if( HasCapture() )
	{
		ReleaseMouse();
	}
	event.Skip();
}

void FxAudioWidget::OnGetFocus( wxFocusEvent& FxUnused(event) )
{
	_hasFocus = FxTrue;
	Refresh(FALSE);
}

void FxAudioWidget::OnLoseFocus( wxFocusEvent& FxUnused(event) )
{
	_hasFocus = FxFalse;
	Refresh(FALSE);
}

void FxAudioWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Animation Tab"));
}

void FxAudioWidget::_zoom( const wxPoint& zoomPoint, FxReal zoomFactor, 
						  FxZoomDir zoomDir )
{
	if( zoomDir != ZD_None )
	{
		wxRect clientRect = GetClientRect();
		FxReal timeSpan = _maxTime - _minTime;
		FxReal time  = FxTimeManager::CoordToTime( zoomPoint.x, clientRect );
		FxReal timePct = (time - _minTime) / timeSpan;

		// Zoom in/out around the point by the 'zoom factor'.
		FxReal newTimeSpan = (zoomDir == ZD_In) 
			? (timeSpan * zoomFactor) : (timeSpan / zoomFactor);

		// Recalculate the minimum and maximum times.
		FxReal tempMinTime = time - ( newTimeSpan * timePct );
		FxReal tempMaxTime = time + ( newTimeSpan * (1 - timePct ) );
		FxTimeManager::RequestTimeChange( tempMinTime, tempMaxTime );
	}
}

void FxAudioWidget::_restoreDefaults( void )
{
	_setWindowLength( kDefaultWindowLength );
	_windowFnType = kDefaultWindowFunctionType;
	_waveformBackgroundColor = kDefaultWaveformBackgroundColor;
	_lowEnergyColor             = kDefaultLowEnergyColor;
	_highEnergyColor            = kDefaultHighEnergyColor;
	_reverseSpectralColors      = kDefaultReverseSpectralColors;
	_colorGamma                 = kDefaultColorGamma;
	_plotType                   = kDefaultPlotType;
	_waveformBackgroundColor    = kDefaultWaveformBackgroundColor;
	_waveformPlotColor          = kDefaultWaveformPlotColor;
	_waveformGridColor          = kDefaultWaveformGridColor;
	_waveformThresholdLineColor = kDefaultWaveformThresholdLineColor;
	_waveformZeroAxisColor      = kDefaultWaveformZeroAxisColor;
	_textColor					= kDefaultTextColor;
	_timeAxisOnTop				= kDefaultTimeAxisPos;
	_mouseWheelZoomFactor       = kDefaultMouseWheelZoomFactor;
}

//------------------------------------------------------------------------------
// The audio view options dialog.
//------------------------------------------------------------------------------
WX_IMPLEMENT_DYNAMIC_CLASS( FxAudioWidgetOptionsDialog, wxDialog )

BEGIN_EVENT_TABLE( FxAudioWidgetOptionsDialog, wxDialog )
	EVT_BUTTON( ControlID_AudioWidgetOptionsDlgApplyButton, FxAudioWidgetOptionsDialog::OnApply )
	EVT_BUTTON( ControlID_AudioWidgetOptionsDlgRestoreDefaultsButton, FxAudioWidgetOptionsDialog::OnRestoreDefaults )
	EVT_BUTTON( ControlID_AudioWidgetOptionsDlgChangeGeneralColorButton, FxAudioWidgetOptionsDialog::OnGeneralColorChange )
	EVT_BUTTON( ControlID_AudioWidgetOptionsDlgChangeWaveformColorButton, FxAudioWidgetOptionsDialog::OnWaveformColorChange )
	EVT_BUTTON( ControlID_AudioWidgetOptionsDlgChangeSpectralColorButton, FxAudioWidgetOptionsDialog::OnSpectralColorChange )
	EVT_LISTBOX( ControlID_AudioWidgetOptionsDlgGeneralColorsListBox, FxAudioWidgetOptionsDialog::OnGeneralColorsSelectionChange )
	EVT_LISTBOX( ControlID_AudioWidgetOptionsDlgWaveformColorsListBox, FxAudioWidgetOptionsDialog::OnWaveformColorsSelectionChange )
	EVT_LISTBOX( ControlID_AudioWidgetOptionsDlgSpectralColorsListBox, FxAudioWidgetOptionsDialog::OnSpectralColorsSelectionChange )
	EVT_CHECKBOX( ControlID_AudioWidgetOptionsDlgReverseSpectralCheck, FxAudioWidgetOptionsDialog::OnReverseSpectralChange )
	EVT_SLIDER( ControlID_AudioWidgetOptionsDlgMouseWheelZoomFactorSlider, FxAudioWidgetOptionsDialog::OnMouseWheelZoomFactorChange )
	EVT_SLIDER( ControlID_AudioWidgetOptionsDlgMousePanningSpeedSlider, FxAudioWidgetOptionsDialog::OnMousePanningSpeedChange )
	EVT_SLIDER( ControlID_AudioWidgetOptionsDlgColorGammaSlider, FxAudioWidgetOptionsDialog::OnColorGammaChange )
	EVT_CHOICE( ControlID_AudioWidgetOptionsDlgWindowLengthChoice, FxAudioWidgetOptionsDialog::OnWindowLengthChange )
	EVT_CHOICE( ControlID_AudioWidgetOptionsDlgWindowFunctionChoice, FxAudioWidgetOptionsDialog::OnWindowFunctionChange )
	EVT_CHECKBOX( ControlID_AudioWidgetOptionsDlgTimeAxisOnTopCheck, FxAudioWidgetOptionsDialog::OnTimeAxisOnTopChange )
END_EVENT_TABLE()

wxColour FxAudioWidgetOptionsDialog::
_customColors[FxAudioWidgetOptionsDialog::_numCustomColors];

FxAudioWidgetOptionsDialog::FxAudioWidgetOptionsDialog()
: _audioView(NULL)
, _applyButton(NULL)
, _spectralColorListBox(NULL)
, _spectralColorPreview(NULL)
, _windowLengthChoice(NULL)
, _windowFunctionChoice(NULL)
, _waveformColorListBox(NULL)
, _waveformColorPreview(NULL)
, _generalColorListBox(NULL)
, _generalColorPreview(NULL)
{
}

FxAudioWidgetOptionsDialog::
FxAudioWidgetOptionsDialog( wxWindow* parent, wxWindowID id, 
						   const wxString& caption, const wxPoint& pos, 
						   const wxSize& size, long style )
{
	Create( parent, id, caption, pos, size, style );
}

FxBool FxAudioWidgetOptionsDialog::
Create( wxWindow* parent, wxWindowID id, const wxString& caption, 
	   const wxPoint& pos, const wxSize& size, long style )
{
	_audioView = static_cast<FxAudioWidget*>(parent);

	_getOptions();

	SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
	SetExtraStyle(GetExtraStyle()|wxWS_EX_VALIDATE_RECURSIVELY);
	wxDialog::Create( parent, id, caption, pos, size, style );

	SetAffirmativeId(ControlID_AudioWidgetOptionsDlgOKButton);
	SetEscapeId(ControlID_AudioWidgetOptionsDlgCancelButton);

	CreateControls();
	GetSizer()->Fit(this);
	GetSizer()->SetSizeHints(this);
	Centre();
	return TRUE;
}

void FxAudioWidgetOptionsDialog::CreateControls( void )
{
	wxBoxSizer* boxSizerV = new wxBoxSizer( wxVERTICAL );
	this->SetSizer(boxSizerV);
	this->SetAutoLayout(TRUE);

	wxNotebook* notebook = new wxNotebook( this, -1, wxDefaultPosition, wxSize(375, 220), wxNB_TOP );
	boxSizerV->Add(notebook, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	// General options.
	wxPanel* generalPanel = new wxPanel( notebook, -1 );

	wxBoxSizer* generalPanelSizer = new wxBoxSizer( wxVERTICAL );
	generalPanel->SetSizer(generalPanelSizer);
	generalPanel->SetAutoLayout(TRUE);
	wxGridSizer* generalPanelSliderSizer = new wxGridSizer( 2, 2, 0, 0 );
	generalPanelSizer->Add(generalPanelSliderSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);
	wxStaticText* mouseWheelZoomFactorLabel = new wxStaticText( generalPanel, wxID_STATIC, _("Mouse wheel zoom factor") );
	generalPanelSliderSizer->Add(mouseWheelZoomFactorLabel, 0, wxALIGN_LEFT|wxALL|wxADJUST_MINSIZE, 5);
	wxSlider* mouseWheelZoomFactorSlider = new wxSlider( generalPanel, ControlID_AudioWidgetOptionsDlgMouseWheelZoomFactorSlider, 0, 101, 500, wxDefaultPosition, wxSize(175,20), wxSL_HORIZONTAL, wxGenericValidator(&_mouseWheelZoomFactor) );
	generalPanelSliderSizer->Add(mouseWheelZoomFactorSlider, 0, wxALIGN_LEFT|wxALL, 5);
	wxCheckBox* timeAxisPos = new wxCheckBox( generalPanel, ControlID_AudioWidgetOptionsDlgReverseSpectralCheck, _("Time axis on top"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_timeAxisOnTop) );
	generalPanelSliderSizer->Add(timeAxisPos, 0, wxALIGN_LEFT|wxALL, 5);
	wxBoxSizer* generalPanelColorSizer = new wxBoxSizer( wxHORIZONTAL );
	generalPanelSizer->Add(generalPanelColorSizer, 0, wxALIGN_LEFT|wxALL, 5);
	static const FxInt32 numGeneralColorChoices = 1;
	wxString generalColorChoices[numGeneralColorChoices];
	generalColorChoices[0] = _("Text color");
	_generalColorListBox = new wxListBox( generalPanel, ControlID_AudioWidgetOptionsDlgWaveformColorsListBox, wxDefaultPosition, wxDefaultSize, numGeneralColorChoices, generalColorChoices );
	generalPanelColorSizer->Add(_generalColorListBox, 0, wxALIGN_LEFT|wxALL, 5);
	wxBoxSizer* generalPanelColorChangeSizer = new wxBoxSizer( wxVERTICAL );
	generalPanelColorSizer->Add(generalPanelColorChangeSizer, 0, wxALIGN_LEFT|wxALL, 5);
	wxBitmap generalColorPreviewBitmap( wxNullBitmap );
	_generalColorPreview = new wxStaticBitmap( generalPanel, wxID_STATIC, generalColorPreviewBitmap, wxDefaultPosition, wxSize(92,20), 0 );
	generalPanelColorChangeSizer->Add(_generalColorPreview, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* changeGeneralColorButton = new wxButton(generalPanel, ControlID_AudioWidgetOptionsDlgChangeGeneralColorButton, _("&Change color..."), wxDefaultPosition, wxDefaultSize, 0 );
	generalPanelColorChangeSizer->Add(changeGeneralColorButton, 0, wxALIGN_LEFT|wxALL, 5);
	// Force the validators to be updated.
	generalPanel->InitDialog();
	_generalColorPreview->SetBackgroundColour( _textColor );

	notebook->AddPage(generalPanel, _("General"), TRUE);

	// Waveform options.
	wxPanel* waveformPanel = new wxPanel( notebook, -1 );

	wxBoxSizer* waveformPanelSizer = new wxBoxSizer( wxHORIZONTAL );
	waveformPanel->SetSizer(waveformPanelSizer);
	waveformPanel->SetAutoLayout(TRUE);
	static const FxInt32 numWaveformColorChoices = 5;
	wxString waveformColorChoices[numWaveformColorChoices];
	waveformColorChoices[0] = _("Background color");
	waveformColorChoices[1] = _("Plot color");
	waveformColorChoices[2] = _("Grid color");
	waveformColorChoices[3] = _("Threshold line color");
	waveformColorChoices[4] = _("Zero axis color");
	_waveformColorListBox = new wxListBox( waveformPanel, ControlID_AudioWidgetOptionsDlgWaveformColorsListBox, wxDefaultPosition, wxDefaultSize, numWaveformColorChoices, waveformColorChoices );
	waveformPanelSizer->Add(_waveformColorListBox, 0, wxALIGN_LEFT|wxALL, 5);
	wxBoxSizer* waveformPanelSizer2 = new wxBoxSizer( wxVERTICAL );
	waveformPanelSizer->Add(waveformPanelSizer2, 0, wxALIGN_LEFT|wxALL, 5);
	wxBitmap waveformColorPreviewBitmap( wxNullBitmap );
	_waveformColorPreview = new wxStaticBitmap( waveformPanel, wxID_STATIC, waveformColorPreviewBitmap, wxDefaultPosition, wxSize(92,20), 0 );
	waveformPanelSizer2->Add(_waveformColorPreview, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* changeWaveformColorButton = new wxButton(waveformPanel, ControlID_AudioWidgetOptionsDlgChangeWaveformColorButton, _("&Change color..."), wxDefaultPosition, wxDefaultSize, 0 );
	waveformPanelSizer2->Add(changeWaveformColorButton, 0, wxALIGN_LEFT|wxALL, 5);
	// Force the validators to be updated.
	waveformPanel->InitDialog();
	_waveformColorPreview->SetBackgroundColour( _waveformBackgroundColor );

	notebook->AddPage( waveformPanel, _("Waveform") );

	// Spectral options.
	wxPanel* spectralPanel = new wxPanel( notebook, -1 );

	wxBoxSizer* spectralPanelSizer = new wxBoxSizer( wxHORIZONTAL );
	spectralPanel->SetSizer(spectralPanelSizer);
	spectralPanel->SetAutoLayout(TRUE);
	wxBoxSizer* spectralOptionsSizer = new wxBoxSizer( wxVERTICAL );
	spectralPanelSizer->Add(spectralOptionsSizer, 0, wxALIGN_LEFT|wxALL, 5);
	wxStaticText* windowLengthLabel = new wxStaticText( spectralPanel, wxID_STATIC, _("Window length") );
	spectralOptionsSizer->Add(windowLengthLabel, 0, wxALIGN_LEFT|wxALL, 5);
	static const FxInt32 numWindowLengths = 11;
	wxString windowLengthChoices[numWindowLengths];
	windowLengthChoices[0] = _("16");
	windowLengthChoices[1] = _("32");
	windowLengthChoices[2] = _("64");
	windowLengthChoices[3] = _("128");
	windowLengthChoices[4] = _("256");
	windowLengthChoices[5] = _("512");
	windowLengthChoices[6] = _("1024");
	windowLengthChoices[7] = _("2048");
	windowLengthChoices[8] = _("8192");
	windowLengthChoices[9] = _("16384");
	_windowLengthChoice = new wxChoice( spectralPanel, ControlID_AudioWidgetOptionsDlgWindowLengthChoice, wxDefaultPosition, wxDefaultSize, numWindowLengths, windowLengthChoices, 0, wxGenericValidator(&_windowLength) );
	spectralOptionsSizer->Add(_windowLengthChoice, 0, wxALIGN_LEFT|wxALL, 5);
	wxStaticText* windowFunctionLabel = new wxStaticText( spectralPanel, wxID_STATIC, _("Window function") );
	spectralOptionsSizer->Add(windowFunctionLabel, 0, wxALIGN_LEFT|wxALL, 5);
	static const FxInt32 numWindowFunctions = 7;
	wxString windowFunctionChoices[numWindowFunctions];
	windowFunctionChoices[0] = _("WT_Bartlett");
	windowFunctionChoices[1] = _("WT_Blackman");
	windowFunctionChoices[2] = _("WT_BlackmanHarris");
	windowFunctionChoices[3] = _("WT_Hamming");
	windowFunctionChoices[4] = _("WT_Hann");
	windowFunctionChoices[5] = _("WT_Rectangle");
	windowFunctionChoices[6] = _("WT_Welch");
	_windowFunctionChoice = new wxChoice( spectralPanel, ControlID_AudioWidgetOptionsDlgWindowFunctionChoice, wxDefaultPosition, wxDefaultSize, numWindowFunctions, windowFunctionChoices, 0, wxGenericValidator(&_windowFnType) );
	spectralOptionsSizer->Add(_windowFunctionChoice, 0, wxALIGN_LEFT|wxALL, 5);
	wxBoxSizer* spectralColorSizer = new wxBoxSizer( wxVERTICAL );
	spectralPanelSizer->Add(spectralColorSizer, 0, wxALIGN_LEFT|wxALL, 5);
	wxBoxSizer* spectralColorListSizer = new wxBoxSizer( wxHORIZONTAL );
	spectralColorSizer->Add(spectralColorListSizer, 0, wxALIGN_LEFT|wxALL, 5);
	static const FxInt32 numSpectralColorChoices = 2;
	wxString spectralColorChoices[numSpectralColorChoices];
	spectralColorChoices[0] = _("Low energy color");
	spectralColorChoices[1] = _("High energy color");
	_spectralColorListBox = new wxListBox( spectralPanel, ControlID_AudioWidgetOptionsDlgSpectralColorsListBox, wxDefaultPosition, wxDefaultSize, numSpectralColorChoices, spectralColorChoices );
	spectralColorListSizer->Add(_spectralColorListBox, 0, wxALIGN_LEFT|wxALL, 5);
	wxBoxSizer* spectralChangeColorSizer = new wxBoxSizer( wxVERTICAL );
	spectralColorListSizer->Add(spectralChangeColorSizer, 0, wxALIGN_LEFT|wxALL, 5);
	wxBitmap spectralColorPreviewBitmap( wxNullBitmap );
	_spectralColorPreview = new wxStaticBitmap( spectralPanel, wxID_STATIC, spectralColorPreviewBitmap, wxDefaultPosition, wxSize(92,20), 0 );
	spectralChangeColorSizer->Add(_spectralColorPreview, 0, wxALIGN_LEFT|wxALL, 5);
	wxButton* changeSpectralColorButton = new wxButton(spectralPanel, ControlID_AudioWidgetOptionsDlgChangeSpectralColorButton, _("&Change color..."), wxDefaultPosition, wxDefaultSize, 0 );
	spectralChangeColorSizer->Add(changeSpectralColorButton, 0, wxALIGN_LEFT|wxALL, 5);
	wxCheckBox* reverseSpectral = new wxCheckBox( spectralPanel, ControlID_AudioWidgetOptionsDlgReverseSpectralCheck, _("Reverse spectral colors"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_reverseSpectralColors) );
	spectralColorSizer->Add(reverseSpectral, 0, wxALIGN_LEFT|wxALL, 5);
	wxBoxSizer* colorGammaSizer = new wxBoxSizer( wxHORIZONTAL );
	spectralColorSizer->Add(colorGammaSizer, 0, wxALIGN_LEFT|wxALL, 5);
	wxStaticText* colorGammaLabel = new wxStaticText(spectralPanel, wxID_STATIC, _("Color gamma") );
	colorGammaSizer->Add(colorGammaLabel, 0, wxALIGN_LEFT|wxALL, 5);
	wxSlider* colorGammaSlider = new wxSlider(spectralPanel, ControlID_AudioWidgetOptionsDlgColorGammaSlider, 0, -800, 0, wxDefaultPosition, wxSize(175,20), wxSL_HORIZONTAL, wxGenericValidator(&_colorGamma) );
	colorGammaSizer->Add(colorGammaSlider, 0, wxALIGN_LEFT|wxALL, 5);

	// Force the validators to be updated.
	spectralPanel->InitDialog();
	_spectralColorPreview->SetBackgroundColour( _lowEnergyColor );

	notebook->AddPage( spectralPanel, _("Spectral") );

	wxBoxSizer* boxSizerH2 = new wxBoxSizer( wxHORIZONTAL );
	boxSizerV->Add(boxSizerH2, 0, wxALIGN_RIGHT|wxALL, 5);
	wxButton* restoreButton = new wxButton( this, ControlID_AudioWidgetOptionsDlgRestoreDefaultsButton, _("&Restore defaults"), wxDefaultPosition, wxDefaultSize, 0 );
	boxSizerH2->Add(restoreButton, 0, wxALIGN_RIGHT|wxALL, 5);

	wxBoxSizer* boxSizerH = new wxBoxSizer( wxHORIZONTAL );
	boxSizerV->Add(boxSizerH, 0, wxALIGN_RIGHT|wxALL, 5);

	wxButton* okButton = new wxButton( this, ControlID_AudioWidgetOptionsDlgOKButton, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	okButton->SetDefault();
	boxSizerH->Add(okButton, 0, wxALIGN_RIGHT|wxALL, 5);
	wxButton* cancelButton = new wxButton( this, ControlID_AudioWidgetOptionsDlgCancelButton, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	boxSizerH->Add(cancelButton, 0, wxALIGN_RIGHT|wxALL, 5);
	_applyButton = new wxButton( this, ControlID_AudioWidgetOptionsDlgApplyButton, _("&Apply"), wxDefaultPosition, wxDefaultSize, 0 );
	_applyButton->Enable(FALSE);
	boxSizerH->Add(_applyButton, 0, wxALIGN_RIGHT|wxALL, 5);
}

FxBool FxAudioWidgetOptionsDialog::ShowToolTips( void )
{
	return FxTrue;
}

void FxAudioWidgetOptionsDialog::SetOptions( void )
{
	_audioView->_mouseWheelZoomFactor  = static_cast<FxReal>(_mouseWheelZoomFactor / 100.0f);
	// Be smart about setting the options.  Only cause a re-computation if it
	// is absolutely required.
	FxBool reverseSpectralColors = _reverseSpectralColors ? FxTrue : FxFalse;
	FxDReal colorGamma           = static_cast<FxDReal>(-_colorGamma / 100.0f );
	FxColor lowEnergyColor       = FxColor( _lowEnergyColor.Red(), 
		_lowEnergyColor.Green(), 
		_lowEnergyColor.Blue() );
	FxColor highEnergyColor      = FxColor( _highEnergyColor.Red(),
		_highEnergyColor.Green(),
		_highEnergyColor.Blue() );
	if( _audioView->_reverseSpectralColors != reverseSpectralColors ||
		_audioView->_colorGamma            != colorGamma            ||
		_audioView->_lowEnergyColor        != lowEnergyColor        ||
		_audioView->_highEnergyColor       != highEnergyColor )
	{
		_audioView->_reverseSpectralColors = reverseSpectralColors;
		_audioView->_colorGamma            = colorGamma;
		_audioView->_lowEnergyColor        = lowEnergyColor;
		_audioView->_highEnergyColor       = highEnergyColor;
		_audioView->_computeColorTable();
	}

	// Plus four to pick the right enum in the range 0 to WL_Last.
	if( _audioView->_windowLength != 1<<(_windowLength+4) ||
		_audioView->_windowFnType != _windowFnType )
	{
		_audioView->_setWindowLength(static_cast<FxAudioWidget::FxWindowLength>(_windowLength+4));
		_audioView->_windowFnType = static_cast<FxFFT::FxFFTWindowType>(_windowFnType);
		_audioView->_computePowerSpectralDensity();
	}

	_audioView->_waveformBackgroundColor    = FxColor(_waveformBackgroundColor.Red(),
		_waveformBackgroundColor.Green(),
		_waveformBackgroundColor.Blue() );
	_audioView->_waveformPlotColor          = FxColor(_waveformPlotColor.Red(),
		_waveformPlotColor.Green(),
		_waveformPlotColor.Blue() );
	_audioView->_waveformGridColor          = FxColor(_waveformGridColor.Red(),
		_waveformGridColor.Green(),
		_waveformGridColor.Blue() );
	_audioView->_waveformThresholdLineColor = FxColor(_waveformThresholdLineColor.Red(),
		_waveformThresholdLineColor.Green(),
		_waveformThresholdLineColor.Blue() );
	_audioView->_waveformZeroAxisColor      = FxColor(_waveformZeroAxisColor.Red(),
		_waveformZeroAxisColor.Green(),
		_waveformZeroAxisColor.Blue() );
	_audioView->_textColor                  = FxColor(_textColor.Red(),
		_textColor.Green(),
		_textColor.Blue() );
	FxBool timeAxisOnTop = _timeAxisOnTop ? FxTrue : FxFalse;
	_audioView->_timeAxisOnTop = timeAxisOnTop;

	// Force the audio view to be repainted.
	_audioView->Refresh(FALSE);
}

void FxAudioWidgetOptionsDialog::_getOptions( void )
{
	// Convert the window length to an index in the choice.
	switch( _audioView->_windowLength )
	{
	case 16:
		_windowLength = 0;
		break;
	case 32:
		_windowLength = 1;
		break;
	case 64:
		_windowLength = 2;
		break;
	case 128:
		_windowLength = 3;
		break;
	case 256:
		_windowLength = 4;
		break;
	default:
	case 512:
		_windowLength = 5;
		break;
	case 1024:
		_windowLength = 6;
		break;
	case 2048:
		_windowLength = 7;
		break;
	case 4096:
		_windowLength = 8;
		break;
	case 8192:
		_windowLength = 9;
		break;
	case 16384:
		_windowLength = 10;
		break;
	}
	_windowFnType          = static_cast<FxInt32>(_audioView->_windowFnType);
	_reverseSpectralColors = _audioView->_reverseSpectralColors == FxTrue ? FxTrue : FxFalse;
	_colorGamma            = -_audioView->_colorGamma * 100;
	_mouseWheelZoomFactor  = _audioView->_mouseWheelZoomFactor * 100;
	_lowEnergyColor.Set( 
		_audioView->_lowEnergyColor.GetRedByte(),
		_audioView->_lowEnergyColor.GetGreenByte(),
		_audioView->_lowEnergyColor.GetBlueByte() );
	_highEnergyColor.Set( 
		_audioView->_highEnergyColor.GetRedByte(),
		_audioView->_highEnergyColor.GetGreenByte(),
		_audioView->_highEnergyColor.GetBlueByte() );
	_waveformBackgroundColor.Set( 
		_audioView->_waveformBackgroundColor.GetRedByte(),
		_audioView->_waveformBackgroundColor.GetGreenByte(),
		_audioView->_waveformBackgroundColor.GetBlueByte() );
	_waveformPlotColor.Set(	
		_audioView->_waveformPlotColor.GetRedByte(),
		_audioView->_waveformPlotColor.GetGreenByte(),
		_audioView->_waveformPlotColor.GetBlueByte() );
	_waveformGridColor.Set(	
		_audioView->_waveformGridColor.GetRedByte(),
		_audioView->_waveformGridColor.GetGreenByte(),
		_audioView->_waveformGridColor.GetBlueByte() );
	_waveformThresholdLineColor.Set( 
		_audioView->_waveformThresholdLineColor.GetRedByte(),
		_audioView->_waveformThresholdLineColor.GetGreenByte(),
		_audioView->_waveformThresholdLineColor.GetBlueByte() );
	_waveformZeroAxisColor.Set( 
		_audioView->_waveformZeroAxisColor.GetRedByte(),
		_audioView->_waveformZeroAxisColor.GetGreenByte(),
		_audioView->_waveformZeroAxisColor.GetBlueByte() );
	_textColor.Set( 
		_audioView->_textColor.GetRedByte(),
		_audioView->_textColor.GetGreenByte(),
		_audioView->_textColor.GetBlueByte() );
	_timeAxisOnTop = _audioView->_timeAxisOnTop == FxTrue ? FxTrue : FxFalse;
}

void FxAudioWidgetOptionsDialog::OnApply( wxCommandEvent& FxUnused(event) )
{
	Validate();
	TransferDataFromWindow();
	SetOptions();
	_applyButton->Enable(FALSE);
}

void FxAudioWidgetOptionsDialog::OnRestoreDefaults( wxCommandEvent& event )
{
	// Minus four to pick the right enum in the range 0 to WL_Last.
	_windowLength          = static_cast<FxInt32>(_audioView->kDefaultWindowLength-4);
	_windowFnType          = static_cast<FxInt32>(_audioView->kDefaultWindowFunctionType); 
	_reverseSpectralColors = _audioView->kDefaultReverseSpectralColors == FxTrue ? FxTrue : FxFalse;
	_colorGamma            = -_audioView->kDefaultColorGamma * 100;
	_mouseWheelZoomFactor  = _audioView->kDefaultMouseWheelZoomFactor * 100;
	_lowEnergyColor.Set( 
		_audioView->kDefaultLowEnergyColor.GetRedByte(),
		_audioView->kDefaultLowEnergyColor.GetGreenByte(),
		_audioView->kDefaultLowEnergyColor.GetBlueByte() );
	_highEnergyColor.Set( 
		_audioView->kDefaultHighEnergyColor.GetRedByte(),
		_audioView->kDefaultHighEnergyColor.GetGreenByte(),
		_audioView->kDefaultHighEnergyColor.GetBlueByte() );
	_waveformBackgroundColor.Set( 
		_audioView->kDefaultWaveformBackgroundColor.GetRedByte(),
		_audioView->kDefaultWaveformBackgroundColor.GetGreenByte(),
		_audioView->kDefaultWaveformBackgroundColor.GetBlueByte() );
	_waveformPlotColor.Set(	
		_audioView->kDefaultWaveformPlotColor.GetRedByte(),
		_audioView->kDefaultWaveformPlotColor.GetGreenByte(),
		_audioView->kDefaultWaveformPlotColor.GetBlueByte() );
	_waveformGridColor.Set(	
		_audioView->kDefaultWaveformGridColor.GetRedByte(),
		_audioView->kDefaultWaveformGridColor.GetGreenByte(),
		_audioView->kDefaultWaveformGridColor.GetBlueByte() );
	_waveformThresholdLineColor.Set( 
		_audioView->kDefaultWaveformThresholdLineColor.GetRedByte(),
		_audioView->kDefaultWaveformThresholdLineColor.GetGreenByte(),
		_audioView->kDefaultWaveformThresholdLineColor.GetBlueByte() );
	_waveformZeroAxisColor.Set( 
		_audioView->kDefaultWaveformZeroAxisColor.GetRedByte(),
		_audioView->kDefaultWaveformZeroAxisColor.GetGreenByte(),
		_audioView->kDefaultWaveformZeroAxisColor.GetBlueByte() );
	_textColor.Set( 
		_audioView->kDefaultTextColor.GetRedByte(),
		_audioView->kDefaultTextColor.GetGreenByte(),
		_audioView->kDefaultTextColor.GetBlueByte() );
	_timeAxisOnTop = _audioView->kDefaultTimeAxisPos == FxTrue ? FxTrue : FxFalse;
	Validate();
	TransferDataToWindow();
	// Force the color preview bitmaps to update themselves.
	wxCommandEvent temp;
	OnGeneralColorsSelectionChange( temp );
	OnWaveformColorsSelectionChange( temp );
	OnSpectralColorsSelectionChange( temp );
	_applyButton->Enable(TRUE);
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnMouseWheelZoomFactorChange( wxCommandEvent& event )
{
	_applyButton->Enable(TRUE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnMousePanningSpeedChange( wxCommandEvent& event )
{
	_applyButton->Enable(TRUE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnGeneralColorChange( wxCommandEvent& event )
{
	wxString selectedColor = _generalColorListBox->GetString(_generalColorListBox->GetSelection());

	if( selectedColor == _("Text color") )
	{
		_changeColor( _textColor );
		_generalColorPreview->SetBackgroundColour( _textColor );
	}
	_applyButton->Enable(TRUE);
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnWaveformColorChange( wxCommandEvent& event )
{
	wxString selectedColor =_waveformColorListBox->GetString(_waveformColorListBox->GetSelection());

	if( selectedColor == _("Background color") )
	{
		_changeColor( _waveformBackgroundColor );
		_waveformColorPreview->SetBackgroundColour( _waveformBackgroundColor );
	}
	else if( selectedColor == _("Plot color") )
	{
		_changeColor( _waveformPlotColor );
		_waveformColorPreview->SetBackgroundColour( _waveformPlotColor );
	}
	else if( selectedColor == _("Grid color") )
	{
		_changeColor( _waveformGridColor );
		_waveformColorPreview->SetBackgroundColour( _waveformGridColor );
	}
	else if( selectedColor == _("Threshold line color") )
	{
		_changeColor( _waveformThresholdLineColor );
		_waveformColorPreview->SetBackgroundColour( _waveformThresholdLineColor );
	}
	else if( selectedColor == _("Zero axis color") )
	{
		_changeColor( _waveformZeroAxisColor );
		_waveformColorPreview->SetBackgroundColour( _waveformZeroAxisColor );
	}
	_applyButton->Enable(TRUE);
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnSpectralColorChange( wxCommandEvent& event )
{
	wxString selectedColor = _spectralColorListBox->GetString(_spectralColorListBox->GetSelection());

	if( selectedColor == _("Low energy color") )
	{
		_changeColor( _lowEnergyColor );
		_spectralColorPreview->SetBackgroundColour( _lowEnergyColor );
	}
	else if( selectedColor == _("High energy color") )
	{
		_changeColor( _highEnergyColor );
		_spectralColorPreview->SetBackgroundColour( _highEnergyColor );
	}
	_applyButton->Enable(TRUE);
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnGeneralColorsSelectionChange( wxCommandEvent& event )
{
	wxString selectedColor = _generalColorListBox->GetString(_generalColorListBox->GetSelection());

	if( selectedColor == _("Text color") )
	{
		_generalColorPreview->SetBackgroundColour( _textColor );
	}
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnWaveformColorsSelectionChange( wxCommandEvent& event )
{
	wxString selectedColor = _waveformColorListBox->GetString(_waveformColorListBox->GetSelection());

	if( selectedColor == _("Background color") )
	{
		_waveformColorPreview->SetBackgroundColour( _waveformBackgroundColor );
	}
	else if( selectedColor == _("Plot color") )
	{
		_waveformColorPreview->SetBackgroundColour( _waveformPlotColor );
	}
	else if( selectedColor == _("Grid color") )
	{
		_waveformColorPreview->SetBackgroundColour( _waveformGridColor );
	}
	else if( selectedColor == _("Threshold line color") )
	{
		_waveformColorPreview->SetBackgroundColour( _waveformThresholdLineColor );
	}
	else if( selectedColor == _("Zero axis color") )
	{
		_waveformColorPreview->SetBackgroundColour( _waveformZeroAxisColor );
	}
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnSpectralColorsSelectionChange( wxCommandEvent& event )
{
	wxString selectedColor = _spectralColorListBox->GetString(_spectralColorListBox->GetSelection());

	if( selectedColor == _("Low energy color") )
	{
		_spectralColorPreview->SetBackgroundColour( _lowEnergyColor );
	}
	else if( selectedColor == _("High energy color") )
	{
		_spectralColorPreview->SetBackgroundColour( _highEnergyColor );
	}
	Refresh(FALSE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::_changeColor( wxColour& colorToChange )
{
	wxColourDialog colorPickerDialog( this );
	colorPickerDialog.GetColourData().SetChooseFull(TRUE);
	colorPickerDialog.GetColourData().SetColour(colorToChange);
	// Remember the user's custom colors.
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		colorPickerDialog.GetColourData().SetCustomColour(i, FxColourMap::GetCustomColours()[i]);
	}
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( colorPickerDialog.ShowModal() == wxID_OK )
	{
		colorToChange = colorPickerDialog.GetColourData().GetColour();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
	for( FxSize i = 0; i < FxColourMap::GetNumCustomColours(); ++i )
	{
		FxColourMap::GetCustomColours()[i] = colorPickerDialog.GetColourData().GetCustomColour(i);
	}
}

void FxAudioWidgetOptionsDialog::OnReverseSpectralChange( wxCommandEvent& event )
{
	_applyButton->Enable(TRUE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnColorGammaChange( wxCommandEvent& event )
{
	_applyButton->Enable(TRUE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnWindowLengthChange( wxCommandEvent& event )
{
	_applyButton->Enable(TRUE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnWindowFunctionChange( wxCommandEvent& event )
{
	_applyButton->Enable(TRUE);
	event.Skip();
}

void FxAudioWidgetOptionsDialog::OnTimeAxisOnTopChange( wxCommandEvent& event )
{
	_applyButton->Enable(TRUE);
	event.Skip();
}

} // namespace Face

} // namespace OC3Ent
