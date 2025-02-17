//------------------------------------------------------------------------------
// The gesture widget, a testbed for tuning new speech gesture algorithms.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxGestureWidget.h"
#include "FxAudio.h"
#include "FxAnimUserData.h"
#include "FxCurveUserData.h"
#include "FxValidators.h"
#include "FxArchiveStoreFile.h"
#include "FxArchiveStoreNull.h"
#include "FxStudioApp.h"
#include "FxCGConfigManager.h"
#include "FxTearoffWindow.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/grid.h"
	#include "wx/valgen.h"
#endif

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxGestureWidget, wxWindow)

BEGIN_EVENT_TABLE(FxGestureWidget, wxWindow)
	EVT_BUTTON(ControlID_GestureWidgetProcessButton, FxGestureWidget::OnRegenerateGestures)
	EVT_BUTTON(ControlID_GestureWidgetReinitializeGesturesButton, FxGestureWidget::OnReinitializeGestures)
	EVT_CHOICE(ControlID_GestureWidgetEAAnimChoice, FxGestureWidget::OnEmphasisActionSelChange)
	EVT_BUTTON(ControlID_GestureWidgetUpdateEAAnimButton, FxGestureWidget::OnEmphasisActionUpdateButton)
	EVT_GRID_CMD_CELL_CHANGE(ControlID_GestureWidgetEATracksGrid, FxGestureWidget::OnEmphasisActionTrackGridChange)
	EVT_BUTTON(ControlID_GestureWidgetLoadConfigButton, FxGestureWidget::OnSerializeConfig)
	EVT_BUTTON(ControlID_GestureWidgetSaveConfigButton, FxGestureWidget::OnSerializeConfig)
	EVT_BUTTON(ControlID_GestureWidgetResetDefaultConfigButton, FxGestureWidget::OnResetDefaultConfig)
END_EVENT_TABLE()

FxGestureWidget::FxGestureWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: Super( parent, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxTAB_TRAVERSAL )
	, FxWidget(mediator)
	, _pAnim(NULL)
	, _pPhonList(NULL)
	, _randomSeed(0)
{
	_config.SetName("Default");
	CreateControls();
}

FxGestureWidget::~FxGestureWidget()
{
}

void FxGestureWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* FxUnused(actor) )
{
	// Update the widget
	_timeFormat = FxStudioOptions::GetTimelineGridFormat();
	TransferDataToWindow();
	FillEAGrids();
	FillODGrid();
	FillConfigName();
}

void FxGestureWidget::OnAnimChanged( FxWidget* FxUnused(sender), const FxName& animGroupName, FxAnim* anim )
{
	_pAnim = anim;
	_animGroupName = animGroupName;
	_pPhonList = NULL;
	if( _pAnim )
	{
		FxAnimUserData* pAnimUserData = reinterpret_cast<FxAnimUserData*>(_pAnim->GetUserData());
		if( pAnimUserData )
		{
			_pPhonList = pAnimUserData->GetPhonemeWordListPointer();
			_pAudio = pAnimUserData->GetAudioPointer();

			// Read a copy of the config from the user data.
			_config = FxGestureConfig(*FxCGConfigManager::GetGestureConfig(pAnimUserData->GetGestureConfig().GetData()));
			TransferDataToWindow();
			FillEAGrids();
			FillODGrid();
			FillConfigName();
		}
	}
}

void FxGestureWidget::OnAnimPhonemeListChanged( FxWidget* FxUnused(sender), FxAnim* FxUnused(anim), FxPhonWordList* phonemeList )
{
	_pPhonList = phonemeList;
}

void FxGestureWidget::OnCreateAnim( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* FxUnused(anim) )
{
}

void FxGestureWidget::OnRecalculateGrid( FxWidget* FxUnused(sender) )
{
	_timeFormat = FxStudioOptions::GetTimelineGridFormat();
	TransferDataToWindow();
	FillEAGrids();
	FillODGrid();
	FillConfigName();
}

void FxGestureWidget::OnGestureConfigChanged( FxWidget* FxUnused(sender), FxGestureConfig* config )
{
	if( config )
	{
		_config = (*config);
		TransferDataToWindow();
		FillEAGrids();
		FillODGrid();
		FillConfigName();
	}
}

void FxGestureWidget::OnRegenerateGestures( wxCommandEvent& FxUnused(event) )
{
	ReadEAGrids();
	ReadODGrid();
	ReadConfigName();
	TransferDataFromWindow();

	if( _pAnim && _pPhonList )
	{
		// Cache the current phoneme list.
		FxPhonWordList cachedPhonList = *_pPhonList;

		FxPhonemeList genericPhonList;
		cachedPhonList.ToGenericFormat(genericPhonList);

		_speechGestures.SetConfig(_config);
		_speechGestures.Initialize(_pAudio->GetSamples(), _pAudio->GetNumSamples(), _pAudio->GetSampleRate());
		_speechGestures.Align(&genericPhonList, _randomSeed);

		FxAnim* pAnim = _pAnim;
		FxName tempGroupName = _animGroupName;
		if( _mediator )
		{
			// Deselect the animation to destroy user data.
			_mediator->OnAnimChanged(NULL, FxName::NullName, NULL);
		}

		for( FxInt32 track = GT_First; track <= GT_Last; ++track )
		{
			const FxAnimCurve& curve = _speechGestures.GetTrack(static_cast<FxGestureTrack>(track));
			pAnim->RemoveAnimCurve(curve.GetName());
			pAnim->AddAnimCurve(curve);
		}

		if( _mediator )
		{
			_mediator->OnActorInternalDataChanged(this, ADCF_CurvesChanged);
			// Reselect the animation to create user data.
			_mediator->OnAnimChanged(NULL, tempGroupName, pAnim);
			*_pPhonList = cachedPhonList;
			// Reset the phoneme list.
			_mediator->OnAnimPhonemeListChanged(this, pAnim, _pPhonList);
		}
	}
}

void FxGestureWidget::OnReinitializeGestures( wxCommandEvent& event )
{
	OnRegenerateGestures(event);
}

void FxGestureWidget::OnEmphasisActionSelChange( wxCommandEvent& FxUnused(event) )
{
	_eaUpdateButton->Enable(FxFalse);
	FillTrackGrid();
}

void FxGestureWidget::OnEmphasisActionUpdateButton( wxCommandEvent& FxUnused(event) )
{
	ReadTrackGrid();
	_eaUpdateButton->Enable(FxFalse);
}

void FxGestureWidget::OnEmphasisActionTrackGridChange( wxGridEvent& FxUnused(event) )
{
	_eaUpdateButton->Enable(FxTrue);
}

void FxGestureWidget::OnSerializeConfig( wxCommandEvent& event )
{
	FxArchive::FxArchiveMode mode = event.GetId() == ControlID_GestureWidgetSaveConfigButton ? FxArchive::AM_Save : FxArchive::AM_Load;

	if( mode == FxArchive::AM_Save )
	{
		ReadEAGrids();
		ReadODGrid();
		ReadConfigName();
		TransferDataFromWindow();
	}

	FxInt32 fileDlgMode = mode == FxArchive::AM_Save ? wxFD_SAVE|wxFD_OVERWRITE_PROMPT : wxFD_OPEN;
	wxString fileDlgTitle = mode == FxArchive::AM_Save ? _("Save FaceFX Gesture Config") : _("Load FaceFX Gesture Config");
	wxFileName defaultPath(FxStudioApp::GetAppDirectory());
	defaultPath.AppendDir(wxT("Configs"));
	defaultPath.AppendDir(wxT("Gestures"));
	wxString fileDlgFile = wxEmptyString;
	if( mode == FxArchive::AM_Save )
	{
		fileDlgFile = wxString::FromAscii(_config.GetNameAsCstr()) + wxT(".fxg");
	}
	wxFileDialog fileDlg(this, fileDlgTitle, defaultPath.GetPath(), fileDlgFile, _("FaceFX Gesture Config (*.fxg)|*.fxg|All Files (*.*)|*.*"), fileDlgMode);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( fileDlg.ShowModal() == wxID_OK )
	{
		FxString filePath = FxString(fileDlg.GetPath().mb_str(wxConvLibc));
		if( mode == FxArchive::AM_Load )
		{
			FxLoadGestureConfigFromFile(_config, filePath.GetData(), FxTrue);
		}
		else
		{
			FxSaveGestureConfigToFile(_config, filePath.GetData());
		}
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();

	if( mode == FxArchive::AM_Load )
	{
		TransferDataToWindow();
		FillEAGrids();
		FillODGrid();
		FillConfigName();
	}
	else
	{
		// We've saved, so we reload the changes.
		FxCGConfigManager::ScanAndLoadAllConfigs();
		if( _mediator )
		{
			// Tell the system to reload the current config.
			_mediator->OnGestureConfigChanged(this, NULL);
		}
	}
}

void FxGestureWidget::OnResetDefaultConfig( wxCommandEvent& FxUnused(event) )
{
	_config = FxGestureConfig();
	TransferDataToWindow();
	FillEAGrids();
	FillODGrid();
	FillConfigName();
}

wxString FxGestureWidget::GetName( FxEmphasisAction emphasisAction )
{
	switch( emphasisAction )
	{
	case EA_StrongHeadNod:
		return _("Strong Head Nod");
	case EA_InvertedHeadNod:
		return _("Inverted Head Nod");
	case EA_QuickHeadNod:
		return _("Quick Head Nod");
	case EA_NormalHeadNod:
		return _("Normal Head Nod");
	case EA_EyebrowRaise:
		return _("Eyebrow Raise");
	case EA_HeadTilt:
		return _("Head Tilt");
	case EA_HeadTurn:
		return _("Head Turn");
	case EA_NoAction:
		return _("No Action");
	default:
		return _("Invalid");
	};
}

wxString FxGestureWidget::GetName( FxGestureTrack gestureTrack )
{
	switch( gestureTrack )
	{
	case GT_OrientationHeadPitch:
		return _("Orientation Head Pitch");
	case GT_OrientationHeadRoll:
		return _("Orientation Head Roll");
	case GT_OrientationHeadYaw:
		return _("Orientation Head Yaw");
	case GT_GazeEyePitch:
		return _("Gaze Eye Pitch");
	case GT_GazeEyeYaw:
		return _("Gaze Eye Yaw");
	case GT_EmphasisHeadPitch:
		return _("Emphasis Head Pitch");
	case GT_EmphasisHeadRoll:
		return _("Emphasis Head Roll");
	case GT_EmphasisHeadYaw:
		return _("Emphasis Head Yaw");
	case GT_EyebrowRaise:
		return _("Eyebrow Raise");
	case GT_Blink:
		return _("Blink");
	default:
		return _("Invalid");
	};
}

wxString FxGestureWidget::GetName( FxStressCategory stressCategory )
{
	switch( stressCategory )
	{
	case SC_Initial:
		return _("Initial");
	case SC_Quick:
		return _("Quick");
	case SC_Normal:
		return _("Normal");
	case SC_Isolated:
		return _("Isolated");
	case SC_Final:
		return _("Final");
	default:
		return _("Unknown");
	};
}

void FxGestureWidget::CreateControls()
{
	wxFont unicodeFont( 9.0f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, _("Lucida Sans Unicode") );
	wxString probability((wxChar)8473);
	wxString variance = wxString((wxChar)0x03C3) + wxString((wxChar)0x00B2);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainSizer);

	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	wxButton* loadButton  = new wxButton(this, ControlID_GestureWidgetLoadConfigButton, _("Load Config..."));
	row->Add(loadButton, 0, wxALL, 5);
	wxButton* saveButton  = new wxButton(this, ControlID_GestureWidgetSaveConfigButton, _("Save Config..."));
	row->Add(saveButton, 0, wxALL, 5);
	wxButton* resetButton = new wxButton(this, ControlID_GestureWidgetResetDefaultConfigButton, _("Reset to Default Config"));
	row->Add(resetButton, 0, wxALL, 5);
	wxButton* goButton = new wxButton(this, ControlID_GestureWidgetProcessButton, wxT("Regenerate Speech Gestures"));
	row->Add(goButton, 0, wxALL, 5);
	mainSizer->Add(row, 0, wxALL, 5);

	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Config Name")), 0, wxALL, 5);
	row->Add((_configName = new wxTextCtrl(this, ControlID_GestureWidgetConfigNameText, wxEmptyString)), 0, wxALL, 5);
	mainSizer->Add(row);
	mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Config Description")), 0, wxALL, 5);
	mainSizer->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(350,100), wxTE_MULTILINE, FxStringValidator(&_config.description)), 0, wxALL, 5);

	wxStaticBoxSizer* eaSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Emphasis Actions")), wxVERTICAL);
	wxChoice* eaAnimSel = new wxChoice(this, ControlID_GestureWidgetEAAnimChoice, wxDefaultPosition, wxDefaultSize);
	eaSizer->Add(eaAnimSel, 0, wxALL, 5);
	mainSizer->Add(eaSizer);

	// EA Track Grid
	wxStaticText* eaTrackGridLbl = new wxStaticText(this, wxID_ANY, _("Tracks"));
	eaSizer->Add(eaTrackGridLbl, 0, wxALL, 5);
	wxGrid* eaTrackGrid = new wxGrid(this, ControlID_GestureWidgetEATracksGrid, wxDefaultPosition, wxSize(720,160));
	eaTrackGrid->CreateGrid(GT_NumEmphasisTracks, 8);
	// Label the grid.
	eaTrackGrid->SetRowLabelSize(150);
	for( FxSize i = GT_EmphasisFirst; i <= GT_EmphasisLast; ++i )
	{
		eaTrackGrid->SetRowLabelValue(i - GT_EmphasisFirst, GetName(static_cast<FxGestureTrack>(i)));
	}
	eaTrackGrid->SetDefaultColSize(50);
	eaTrackGrid->SetColLabelValue(0, _("L"));
	eaTrackGrid->SetColLabelValue(1, variance + _("(L)"));
	eaTrackGrid->SetColLabelValue(2, _("C"));
	eaTrackGrid->SetColLabelValue(3, variance + _("(C)"));
	eaTrackGrid->SetColLabelValue(4, _("T"));
	eaTrackGrid->SetColLabelValue(5, variance + _("(T)"));
	eaTrackGrid->SetColLabelValue(6, _("V"));
	eaTrackGrid->SetColLabelValue(7, probability);
	//eaTrackGrid->SetDefaultRenderer(new wxGridCellFloatRenderer(-1, 3));
	eaTrackGrid->SetDefaultEditor(new wxGridCellFloatEditor(-1, 3));
	eaTrackGrid->SetLabelFont(unicodeFont);

	eaSizer->Add(eaTrackGrid, 0, wxALL, 5);
	
	// EA Anim Update Button
	wxButton* eaUpdateButton = new wxButton(this, ControlID_GestureWidgetUpdateEAAnimButton, _("Update"));
	eaUpdateButton->Enable(FALSE);
	eaSizer->Add(eaUpdateButton, 0, wxALL, 5);

	// EA Probabilities and Scale Grid
	wxStaticText* eaProbScaleGridLbl = new wxStaticText(this, wxID_ANY, _("Probabilities and Scaling"));
	eaSizer->Add(eaProbScaleGridLbl, 0, wxALL, 5);
	wxGrid* eaProbScaleGrid = new wxGrid(this, ControlID_GestureWidgetEAProbScaleGrid, wxDefaultPosition, wxSize(720,200));
	eaProbScaleGrid->CreateGrid(EA_NumActions, SC_NumCategories + 2);
	eaProbScaleGrid->SetRowLabelSize(150);
	for( FxSize i = EA_First; i <= EA_Last; ++i )
	{
		eaAnimSel->AppendString(GetName(static_cast<FxEmphasisAction>(i)));
		eaProbScaleGrid->SetRowLabelValue(i, GetName(static_cast<FxEmphasisAction>(i)));
	}
	eaProbScaleGrid->SetDefaultColSize(60);
	for( FxSize i = SC_First; i <= SC_Last; ++i )
	{
		eaProbScaleGrid->SetColLabelValue(i, GetName(static_cast<FxStressCategory>(i)));
	}
	eaProbScaleGrid->SetColLabelValue(5, _("Smin"));
	eaProbScaleGrid->SetColLabelValue(6, _("Smax"));
	eaProbScaleGrid->SetDefaultRenderer(new wxGridCellFloatRenderer(-1, 2));
	eaProbScaleGrid->SetDefaultEditor(new wxGridCellFloatEditor(-1, 2));
	eaProbScaleGrid->SetLabelFont(unicodeFont);

	eaSizer->Add(eaProbScaleGrid, 0, wxALL, 5);
	
	// SD Silence Categorizations
	row = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* sdLongSilenceLbl = new wxStaticText(this, wxID_ANY, _("Long silence:"));
	row->Add(sdLongSilenceLbl, 0, wxALL, 5);
	wxTextCtrl* sdLongSilence = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.sdLongSilence, FxTrue));
	row->Add(sdLongSilence, 0, wxALL, 5);
	wxStaticText* sdShortSilenceLbl = new wxStaticText(this, wxID_ANY, _("Short silence:"));
	row->Add(sdShortSilenceLbl, 0, wxALL, 5);
	wxTextCtrl* sdShortSilence = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.sdShortSilence, FxTrue));
	row->Add(sdShortSilence, 0, wxALL, 5);
	eaSizer->Add(row);

	// SD Stress Categorizations
	row = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* sdQuickStressLimitLbl = new wxStaticText(this, wxID_ANY, _("Quick stress limit:"));
	row->Add(sdQuickStressLimitLbl, 0, wxALL, 5);
	wxTextCtrl* sdQuickStressLimit = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.sdQuickStressLimit, FxTrue));
	row->Add(sdQuickStressLimit, 0, wxALL, 5);
	wxStaticText* sdIsolatedStressLimitLbl = new wxStaticText(this, wxID_ANY, _("Isolated stress limit:"));
	row->Add(sdIsolatedStressLimitLbl, 0, wxALL, 5);
	wxTextCtrl* sdIsolatedStressLimit = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.sdIsolatedStressLimit, FxTrue));
	row->Add(sdIsolatedStressLimit, 0, wxALL, 5);
	eaSizer->Add(row);

	row = new wxBoxSizer(wxHORIZONTAL);
	// EA Gesture Centre Shift
	wxStaticText* eaGestureCentreShiftLbl = new wxStaticText(this, wxID_ANY, _("Gesture centre shift:"));
	row->Add(eaGestureCentreShiftLbl, 0, wxALL, 5);
	wxTextCtrl* eaGestureCentreShift = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.eaCentreShift, FxTrue));
	row->Add(eaGestureCentreShift, 0, wxALL, 5);
	// SD Min Gesture Separation
//	wxStaticText* sdMinGestureSeparationLbl = new wxStaticText(this, wxID_ANY, _("Minimum stress separation:"));
//	row->Add(sdMinGestureSeparationLbl, 0, wxALL, 5);
//	wxTextCtrl* sdMinGestureSeparation = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.sdMinStressSeparation, FxTrue));
//	row->Add(sdMinGestureSeparation, 0, wxALL, 5);
	eaSizer->Add(row);
	
	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Gesture speed:")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.gcSpeed)), 0, wxALL, 5);
	row->Add(new wxStaticText(this, wxID_ANY, _("magnitude:")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.gcMagnitude)), 0, wxALL, 5);
	eaSizer->Add(row);


	// OD State Properties
	wxStaticBoxSizer* odSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Orientation Drift")), wxVERTICAL);
	wxGrid* odPropGrid = new wxGrid(this, ControlID_GestureWidgetODPropGrid, wxDefaultPosition, wxSize(350, 190));
	odPropGrid->CreateGrid(7, 3);
	odPropGrid->SetColLabelValue(0, _("V"));
	odPropGrid->SetColLabelValue(1, variance + _("(V)"));
	odPropGrid->SetColLabelValue(2, probability);
	odPropGrid->SetRowLabelValue(0, _("Head Pitch"));
	odPropGrid->SetRowLabelValue(1, _("Head Roll"));
	odPropGrid->SetRowLabelValue(2, _("Head Yaw"));
	odPropGrid->SetRowLabelValue(3, _("L@ Mouth"));
	odPropGrid->SetRowLabelValue(4, _("L@ Left Eye"));
	odPropGrid->SetRowLabelValue(5, _("L@ Right Eye"));
	odPropGrid->SetRowLabelValue(6, _("L@ Random"));
	odPropGrid->SetDefaultRenderer(new wxGridCellFloatRenderer(-1, 2));
	odPropGrid->SetDefaultEditor(new wxGridCellFloatEditor(-1, 2));
	odPropGrid->SetLabelFont(unicodeFont);
	odSizer->Add(odPropGrid, 0, wxALL, 5);

	// OD Head State Duration
	row = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* odHeadStateMinLbl = new wxStaticText(this, wxID_ANY, _("Head state min:"));
	row->Add(odHeadStateMinLbl, 0, wxALL, 5);
	wxTextCtrl* odHeadStateMin = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.odHeadDuration.rangeMin, FxTrue));
	row->Add(odHeadStateMin, 0, wxALL, 5);
	wxStaticText* odHeadStateMaxLbl = new wxStaticText(this, wxID_ANY, _("max:"));
	row->Add(odHeadStateMaxLbl, 0, wxALL, 5);
	wxTextCtrl* odHeadStateMax = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.odHeadDuration.rangeMax, FxTrue));
	row->Add(odHeadStateMax, 0, wxALL, 5);
	// OD Head State Transition
	wxStaticText* odHeadTransMinLbl = new wxStaticText(this, wxID_ANY, _("Head transition min:"));
	row->Add(odHeadTransMinLbl, 0, wxALL, 5);
	wxTextCtrl* odHeadTransMin = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.odHeadTransition.rangeMin, FxTrue));
	row->Add(odHeadTransMin, 0, wxALL, 5);
	wxStaticText* odHeadTransMaxLbl = new wxStaticText(this, wxID_ANY, _("max:"));
	row->Add(odHeadTransMaxLbl, 0, wxALL, 5);
	wxTextCtrl* odHeadTransMax = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.odHeadTransition.rangeMax, FxTrue));
	row->Add(odHeadTransMax, 0, wxALL, 5);
	odSizer->Add(row);

	// OD Eye State Duration
	row = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* odEyeStateMinLbl = new wxStaticText(this, wxID_ANY, _("Eye state min:"));
	row->Add(odEyeStateMinLbl, 0, wxALL, 5);
	wxTextCtrl* odEyeStateMin = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.odEyeDuration.rangeMin, FxTrue));
	row->Add(odEyeStateMin, 0, wxALL, 5);
	wxStaticText* odEyeStateMaxLbl = new wxStaticText(this, wxID_ANY, _("max:"));
	row->Add(odEyeStateMaxLbl, 0, wxALL, 5);
	wxTextCtrl* odEyeStateMax = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.odEyeDuration.rangeMax, FxTrue));
	row->Add(odEyeStateMax, 0, wxALL, 5);
	// OD Eye State Transition
	wxStaticText* odEyeTransMinLbl = new wxStaticText(this, wxID_ANY, _("Eye transition min:"));
	row->Add(odEyeTransMinLbl, 0, wxALL, 5);
	wxTextCtrl* odEyeTransMin = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.odEyeTransition.rangeMin, FxTrue));
	row->Add(odEyeTransMin, 0, wxALL, 5);
	wxStaticText* odEyeTransMaxLbl = new wxStaticText(this, wxID_ANY, _("max:"));
	row->Add(odEyeTransMaxLbl, 0, wxALL, 5);
	wxTextCtrl* odEyeTransMax = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.odEyeTransition.rangeMax, FxTrue));
	row->Add(odEyeTransMax, 0, wxALL, 5);
	odSizer->Add(row);

	mainSizer->Add(odSizer, 0, wxALL, 5);

	// ROS Phone Length
	wxStaticBoxSizer* rosSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Rate of Speech")), wxVERTICAL);
	row = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* rosPhonMinLbl = new wxStaticText(this, wxID_ANY, _("Avg phoneme length min:"));
	row->Add(rosPhonMinLbl, 0, wxALL, 5);
	wxTextCtrl* rosPhonMin = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.rosAvgPhonemeDuration.rangeMin, FxTrue));
	row->Add(rosPhonMin, 0, wxALL, 5);
	wxStaticText* rosPhonMaxLbl = new wxStaticText(this, wxID_ANY, _("max:"));
	row->Add(rosPhonMaxLbl, 0, wxALL, 5);
	wxTextCtrl* rosPhonMax = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.rosAvgPhonemeDuration.rangeMax, FxTrue));
	row->Add(rosPhonMax, 0, wxALL, 5);
	rosSizer->Add(row);

	// ROS Gesture Scaling
	row = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* rosGestureScaleMinLbl = new wxStaticText(this, wxID_ANY, _("Gesture scale min:"));
	row->Add(rosGestureScaleMinLbl, 0, wxALL, 5);
	wxTextCtrl* rosGestureScaleMin = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.rosTimeScale.rangeMin, wxT("%0.2f")));
	row->Add(rosGestureScaleMin, 0, wxALL, 5);
	wxStaticText* rosGestureScaleMaxLbl = new wxStaticText(this, wxID_ANY, _("max:"));
	row->Add(rosGestureScaleMaxLbl, 0, wxALL, 5);
	wxTextCtrl* rosGestureScaleMax = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.rosTimeScale.rangeMax, wxT("%0.2f")));
	row->Add(rosGestureScaleMax, 0, wxALL, 5);
	rosSizer->Add(row);

	mainSizer->Add(rosSizer, 0, wxALL, 5);

	// Blinks
	wxStaticBoxSizer* blinkSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Blinks")), wxVERTICAL);
	row = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* blinkMinLbl = new wxStaticText(this, wxID_ANY, _("Blink separation min:"));
	row->Add(blinkMinLbl, 0, wxALL, 5);
	wxTextCtrl* blinkMin = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.blinkSeparation.rangeMin, FxTrue));
	row->Add(blinkMin, 0, wxALL, 5);
	wxStaticText* blinkMaxLbl = new wxStaticText(this, wxID_ANY, _("max:"));
	row->Add(blinkMaxLbl, 0, wxALL, 5);
	wxTextCtrl* blinkMax = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.blinkSeparation.rangeMax, FxTrue));
	row->Add(blinkMax, 0, wxALL, 5);
	blinkSizer->Add(row);
	mainSizer->Add(blinkSizer, 0, wxALL, 5);

	// Power and Frequency Extraction
	wxStaticBoxSizer* pfeSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Power and Frequency Extraction")), wxVERTICAL);
	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Power: num adjacent phonemes to consider:")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_config.pfePowerNumAdjacentPhones)), 0, wxALL, 5);
	row->Add(new wxStaticText(this, wxID_ANY, _("max window length:")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.pfePowerMaxWindowLength, FxTrue)), 0, wxALL, 5);
	pfeSizer->Add(row);
	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Stress computation: average power contribution (%):")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.pfeAvgPowerContribution)), 0, wxALL, 5);
	row->Add(new wxStaticText(this, wxID_ANY, _("max power contribution (%):")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.pfeMaxPowerContribution)), 0, wxALL, 5);
	pfeSizer->Add(row);	
	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Long duration bonus (absolute):")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.pfeLongDurationBonus)), 0, wxALL, 5);
	pfeSizer->Add(row);
	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Rate of speech: Num adjacent phonemes to consider:")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_config.pfeRateOfSpeechNumAdjacentPhones)), 0, wxALL, 5);
	pfeSizer->Add(row);
	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Stress threshold:")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.sdStressThreshold)), 0, wxALL, 5);
	pfeSizer->Add(row);
	mainSizer->Add(pfeSizer, 0, wxALL, 5);

	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Random seed (0 for current time):")), 0, wxALL, 5);
	row->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_randomSeed)), 0, wxALL, 5);
	mainSizer->Add(row, 0, wxALL, 5);

	_eaAnimSel = eaAnimSel;
	_eaTrackGrid = eaTrackGrid;
	_eaProbScaleGrid = eaProbScaleGrid;
	_odPropGrid = odPropGrid;
	_eaUpdateButton = eaUpdateButton;

	mainSizer->FitInside(this);
	SetScrollbars(10, 10, 50, 50);
	TransferDataToWindow();
	eaAnimSel->SetSelection(0);	
	FillEAGrids();
	FillODGrid();
	FillConfigName();
}

void FxGestureWidget::FillEAGrids()
{
	FillTrackGrid();
	FillProbScaleGrid();
}

void FxGestureWidget::FillTrackGrid()
{
	// Clear the track grid to zeros.
	for( FxSize i = 0; i < GT_NumEmphasisTracks; ++i )
	{
		for( FxSize j = 0; j < 6; ++j )
		{
			_eaTrackGrid->SetCellValue(FxFormatTime(0.000, _timeFormat), i, j);
		}
		for( FxSize j = 6; j < 8; ++j )
		{
			_eaTrackGrid->SetCellValue(_("0.000"), i, j);
		}
	}

	// Get the selected emphasis action.
	FxSize selAction = _eaAnimSel->GetSelection();
	// Fill out the track grid.
	FxEmphasisActionAnim& eaAnim = _config.eaProperties[selAction];
	for( FxSize i = 0; i < eaAnim.tracks.Length(); ++i )
	{
		FxSize row = eaAnim.tracks[i].track - GT_EmphasisFirst;
		if( eaAnim.tracks[i].track == GT_Blink )
		{
			// The blink spacing is shared for every gesture.
			_eaTrackGrid->SetCellValue(FxFormatTime(_config.blinkLead.centre, _timeFormat), row, 0);
			_eaTrackGrid->SetCellValue(FxFormatTime(_config.blinkLead.range, _timeFormat), row, 1);
			_eaTrackGrid->SetCellValue(FxFormatTime(0.0f, _timeFormat), row, 2);
			_eaTrackGrid->SetCellValue(FxFormatTime(0.0f, _timeFormat), row, 3);
			_eaTrackGrid->SetCellValue(FxFormatTime(_config.blinkTail.centre, _timeFormat), row, 4);
			_eaTrackGrid->SetCellValue(FxFormatTime(_config.blinkTail.range, _timeFormat), row, 5);
		}
		else
		{
			_eaTrackGrid->SetCellValue(FxFormatTime(eaAnim.tracks[i].lead.centre, _timeFormat), row, 0);
			_eaTrackGrid->SetCellValue(FxFormatTime(eaAnim.tracks[i].lead.range, _timeFormat), row, 1);
			_eaTrackGrid->SetCellValue(FxFormatTime(eaAnim.tracks[i].centre.centre, _timeFormat), row, 2);
			_eaTrackGrid->SetCellValue(FxFormatTime(eaAnim.tracks[i].centre.range, _timeFormat), row, 3);
			_eaTrackGrid->SetCellValue(FxFormatTime(eaAnim.tracks[i].tail.centre, _timeFormat), row, 4);
			_eaTrackGrid->SetCellValue(FxFormatTime(eaAnim.tracks[i].tail.range, _timeFormat), row, 5);
		}
		_eaTrackGrid->SetCellValue(wxString::Format(wxT("%.3f"), eaAnim.tracks[i].value), row, 6);
		_eaTrackGrid->SetCellValue(wxString::Format(wxT("%.3f"), eaAnim.tracks[i].probability), row, 7);
	}
}

void FxGestureWidget::FillProbScaleGrid()
{
	// Clear the prob and scale grid to zeros
	for( FxSize i = 0; i < EA_NumActions; ++i )
	{
		for( FxSize j = 0; j < SC_NumCategories; ++j )
		{
			_eaProbScaleGrid->SetCellValue(wxString::Format(wxT("%.2f"), _config.eaWeights[j][i]), i, j);
		}
		_eaProbScaleGrid->SetCellValue(wxString::Format(wxT("%.2f"), _config.eaProperties[i].valueScale.rangeMin), i, 5);
		_eaProbScaleGrid->SetCellValue(wxString::Format(wxT("%.2f"), _config.eaProperties[i].valueScale.rangeMax), i, 6);
	}
}

void FxGestureWidget::FillODGrid()
{
	for( FxSize i = 0; i < _config.odHeadProperties.Length(); ++i )
	{
		_odPropGrid->SetCellValue(wxString::Format(wxT("%.3f"), _config.odHeadProperties[i].valueRange.centre), i, 0);
		_odPropGrid->SetCellValue(wxString::Format(wxT("%.3f"), _config.odHeadProperties[i].valueRange.range), i, 1);
		_odPropGrid->SetCellValue(wxString::Format(wxT("%.2f"), _config.odHeadProperties[i].probability), i, 2);
	}
	for( FxSize i = 0; i < _config.odEyeProperties.Length(); ++i )
	{
		_odPropGrid->SetCellValue(wxString::Format(wxT("%.3f"), _config.odEyeProperties[i].valueRange.centre), i+3, 0);
		_odPropGrid->SetCellValue(wxString::Format(wxT("%.3f"), _config.odEyeProperties[i].valueRange.range), i+3, 1);
		_odPropGrid->SetCellValue(wxString::Format(wxT("%.2f"), _config.odEyeProperties[i].probability), i+3, 2);
	}
}

void FxGestureWidget::FillConfigName()
{
	_configName->SetValue(wxString::FromAscii(_config.GetNameAsCstr()));
}

void FxGestureWidget::ReadEAGrids()
{
	ReadTrackGrid();
	ReadProbScaleGrid();
}

void FxGestureWidget::ReadTrackGrid()
{
	// Get the selected emphasis action.
	FxSize selAction = _eaAnimSel->GetSelection();
	FxEmphasisActionAnim& eaAnim = _config.eaProperties[selAction];

	// To decide if we need to include a track in the EA Anim, look at its value.
	// If it is non-zero, it should be included.  If it is zero, it should not
	// be included, and should be removed if pre-existing.
	for( FxSize i = GT_EmphasisFirst; i <= GT_EmphasisLast; ++i )
	{
		// Pull out all relevant information.
		FxGestureTrack currentTrack = static_cast<FxGestureTrack>(i);
		FxSize row = i - GT_EmphasisFirst;
		FxCentredRange newLead(0.0f, 0.0f);
		FxCentredRange newCentre(0.0f, 0.0f);
		FxCentredRange newTail(0.0f, 0.0f);
		FxReal newValue = 0.0f;
		FxReal newProbability = 0.0f;

		newLead.centre		= FxUnformatTime(_eaTrackGrid->GetCellValue(row, 0), _timeFormat);
		newLead.range		= FxUnformatTime(_eaTrackGrid->GetCellValue(row, 1), _timeFormat);
		newCentre.centre	= FxUnformatTime(_eaTrackGrid->GetCellValue(row, 2), _timeFormat);
		newCentre.range		= FxUnformatTime(_eaTrackGrid->GetCellValue(row, 3), _timeFormat);
		newTail.centre		= FxUnformatTime(_eaTrackGrid->GetCellValue(row, 4), _timeFormat);
		newTail.range		= FxUnformatTime(_eaTrackGrid->GetCellValue(row, 5), _timeFormat);
		newValue			= FxAtof(_eaTrackGrid->GetCellValue(row, 6).mb_str(wxConvLibc));
		newProbability		= FxAtof(_eaTrackGrid->GetCellValue(row, 7).mb_str(wxConvLibc));

		if( currentTrack == GT_Blink )
		{
			_config.blinkLead = newLead;
			_config.blinkTail = newTail;
		}

		// Attempt to find the track.
		FxSize index = FxInvalidIndex;
		for( FxSize j = 0; j < eaAnim.tracks.Length(); ++j )
		{
			if( eaAnim.tracks[j].track == currentTrack )
			{
				index = j;
			}
		}
		
		if( !FxRealEquality(newValue, 0.0f) )
		{
			// The track should exist
			FxEmphasisActionTrack newTrack(currentTrack, newLead, newCentre, newTail, newValue, newProbability);
			if( index == FxInvalidIndex )
			{
				// The track doesn't yet exist. Create it and add it.
				eaAnim.tracks.PushBack(newTrack);
			}
			else
			{
				// The track exists and its index is known. Update it.
				eaAnim.tracks[index] = newTrack;
			}
		}
		else if( index != FxInvalidIndex )
		{
			// The track exists and it shouldn't.  Remove it.
			eaAnim.tracks.Remove(index);
		}
	}
}

void FxGestureWidget::ReadProbScaleGrid()
{
	// Get the probabilities and scales.
	for( FxSize i = 0; i < EA_NumActions; ++i )
	{
		for( FxSize j = 0; j < SC_NumCategories; ++j )
		{
			_config.eaWeights[j][i] = FxAtof(_eaProbScaleGrid->GetCellValue(i, j).mb_str(wxConvLibc));
		}
		_config.eaProperties[i].valueScale.rangeMin = FxAtof(_eaProbScaleGrid->GetCellValue(i, 5).mb_str(wxConvLibc));
		_config.eaProperties[i].valueScale.rangeMax = FxAtof(_eaProbScaleGrid->GetCellValue(i, 6).mb_str(wxConvLibc));
	}
}

void FxGestureWidget::ReadODGrid()
{
	for( FxSize i = 0; i < _config.odHeadProperties.Length(); ++i )
	{
		_config.odHeadProperties[i].valueRange.centre	= FxAtof(_odPropGrid->GetCellValue(i, 0).mb_str(wxConvLibc));
		_config.odHeadProperties[i].valueRange.range	= FxAtof(_odPropGrid->GetCellValue(i, 1).mb_str(wxConvLibc));
		_config.odHeadProperties[i].probability			= FxAtof(_odPropGrid->GetCellValue(i, 2).mb_str(wxConvLibc));
	}
	for( FxSize i = 0; i < _config.odEyeProperties.Length(); ++i )
	{
		_config.odEyeProperties[i].valueRange.centre	= FxAtof(_odPropGrid->GetCellValue(i+3, 0).mb_str(wxConvLibc));
		_config.odEyeProperties[i].valueRange.range		= FxAtof(_odPropGrid->GetCellValue(i+3, 1).mb_str(wxConvLibc));
		_config.odEyeProperties[i].probability			= FxAtof(_odPropGrid->GetCellValue(i+3, 2).mb_str(wxConvLibc));
	}
}

void FxGestureWidget::ReadConfigName()
{
	wxString configName = _configName->GetValue();
	_config.SetName(FxString(configName.mb_str(wxConvLibc)).GetData());
}

} // namespace Face

} // namespace OC3Ent
