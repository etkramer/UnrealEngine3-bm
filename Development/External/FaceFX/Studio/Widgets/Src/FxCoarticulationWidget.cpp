//------------------------------------------------------------------------------
// The coarticulation widget, a testbed for new coarticulation algorithms.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCoarticulationWidget.h"
#include "FxAnimUserData.h"
#include "FxCurveUserData.h"
#include "FxConsole.h"
#include "FxAnimController.h"
#include "FxStudioOptions.h"
#include "FxValidators.h"
#include "FxSessionProxy.h"
#include "FxCurveUserData.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/splitter.h"
	#include "wx/valgen.h"
#endif

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxCoarticulationWidget, wxWindow)

BEGIN_EVENT_TABLE(FxCoarticulationWidget, wxWindow)
	EVT_SIZE(FxCoarticulationWidget::OnSize)
	EVT_GRID_CMD_CELL_CHANGE(ControlID_CoarticulationWidgetParameterGrid, FxCoarticulationWidget::OnCellChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetSuppressionWindowMaxText, FxCoarticulationWidget::OnConfigChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetSuppressionWindowMaxSlopeText, FxCoarticulationWidget::OnConfigChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetSuppressionWindowMinText, FxCoarticulationWidget::OnConfigChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetSuppressionWindowMinSlopeText, FxCoarticulationWidget::OnConfigChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetShortSilenceText, FxCoarticulationWidget::OnConfigChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetLeadInTimeText, FxCoarticulationWidget::OnConfigChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetLeadOutTimeText, FxCoarticulationWidget::OnConfigChange)
	EVT_CHECKBOX(ControlID_CoarticulationWidgetSplitDiphthongCheck, FxCoarticulationWidget::OnConfigChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetDiphthongBoundaryText, FxCoarticulationWidget::OnConfigChange)
	EVT_CHECKBOX(ControlID_CoarticulationWidgetConvertToFlapsCheck, FxCoarticulationWidget::OnConfigChange)
	EVT_TEXT_ENTER(ControlID_CoarticulationWidgetConfigNameText, FxCoarticulationWidget::OnConfigChange)
END_EVENT_TABLE()

FxCoarticulationWidget::FxCoarticulationWidget( wxWindow* parent, FxWidgetMediator* mediator )
	: Super( parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN )
	, FxWidget(mediator)
	, _doCoarticulation(FxTrue)
{
	wxWindow* topPanel = this;

	topPanel->SetSizer(new wxBoxSizer(wxVERTICAL));

	wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
	hSizer->Add(new wxStaticText(this, wxID_ANY, _("Config Name")), 0, wxALL, 5);
	hSizer->Add((_configName = new wxTextCtrl(this, ControlID_CoarticulationWidgetConfigNameText, wxEmptyString)), 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer, 0, wxALL, 5);

	hSizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* label = new wxStaticText(topPanel, wxID_ANY, _("Suppression Window Max"));
	_suppressionWindowMaxCtrl = new wxTextCtrl(topPanel, ControlID_CoarticulationWidgetSuppressionWindowMaxText, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.suppressionWindowMax));
	hSizer->Add(label, 0, wxALL, 5);
	hSizer->Add(_suppressionWindowMaxCtrl, 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer);

	hSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(topPanel, wxID_ANY, _("Suppression Window Max Slope"));
	_suppressionWindowMaxSlopeCtrl = new wxTextCtrl(topPanel, ControlID_CoarticulationWidgetSuppressionWindowMaxSlopeText, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.suppressionWindowMaxSlope));
	hSizer->Add(label, 0, wxALL, 5);
	hSizer->Add(_suppressionWindowMaxSlopeCtrl, 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer);

	hSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(topPanel, wxID_ANY, _("Suppression Window Min"));
	_suppressionWindowMinCtrl = new wxTextCtrl(topPanel, ControlID_CoarticulationWidgetSuppressionWindowMinText, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.suppressionWindowMin));
	hSizer->Add(label, 0, wxALL, 5);
	hSizer->Add(_suppressionWindowMinCtrl, 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer);

	hSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(topPanel, wxID_ANY, _("Suppression Window Min Slope"));
	_suppressionWindowMinSlopeCtrl = new wxTextCtrl(topPanel, ControlID_CoarticulationWidgetSuppressionWindowMinSlopeText, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxRealValidator(&_config.suppressionWindowMinSlope));
	hSizer->Add(label, 0, wxALL, 5);
	hSizer->Add(_suppressionWindowMinSlopeCtrl, 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer);

	hSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(topPanel, wxID_ANY, _("Short silence"));
	_shortSilenceDurationCtrl = new wxTextCtrl(topPanel, ControlID_CoarticulationWidgetShortSilenceText, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.shortSilenceDuration, FxTrue));
	hSizer->Add(label, 0, wxALL, 5);
	hSizer->Add(_shortSilenceDurationCtrl, 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer);

	hSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(topPanel, wxID_ANY, _("Lead in time"));
	_leadInTimeCtrl = new wxTextCtrl(topPanel, ControlID_CoarticulationWidgetLeadInTimeText, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.leadInTime, FxTrue));
	hSizer->Add(label, 0, wxALL, 5);
	hSizer->Add(_leadInTimeCtrl, 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer);

	hSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(topPanel, wxID_ANY, _("Lead out time"));
	_leadOutTimeCtrl = new wxTextCtrl(topPanel, ControlID_CoarticulationWidgetLeadOutTimeText, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.leadOutTime, FxTrue));
	hSizer->Add(label, 0, wxALL, 5);
	hSizer->Add(_leadOutTimeCtrl, 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer);

	_splitDiphthongsCheck = new wxCheckBox(topPanel, ControlID_CoarticulationWidgetSplitDiphthongCheck, _("Split diphthongs into component vowels."), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_config.splitDiphthongs));
	topPanel->GetSizer()->Add(_splitDiphthongsCheck, 0, wxALL, 5);

	hSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(topPanel, wxID_ANY, _("Diphthong minimum boundary"));
	_diphthongBoundaryCtrl = new wxTextCtrl(topPanel, ControlID_CoarticulationWidgetDiphthongBoundaryText, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, FxTimeValidator(&_config.diphthongBoundary, FxTrue));
	hSizer->AddSpacer(25);
	hSizer->Add(label, 0, wxALL, 5);
	hSizer->Add(_diphthongBoundaryCtrl, 0, wxALL, 5);
	topPanel->GetSizer()->Add(hSizer);

	_convertToFlapsCheck = new wxCheckBox(topPanel, ControlID_CoarticulationWidgetConvertToFlapsCheck, _("Convert appropriate consonants to flaps."), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_config.convertToFlaps));
	topPanel->GetSizer()->Add(_convertToFlapsCheck, 0, wxALL, 5);

	_parameterGrid = new wxGrid(topPanel, ControlID_CoarticulationWidgetParameterGrid);
	_parameterGrid->CreateGrid(NUM_PHONS, 2);
	topPanel->GetSizer()->Add(_parameterGrid, 1, wxALL, 5);

	Layout();

	SetDefaultParameters();
}

FxCoarticulationWidget::~FxCoarticulationWidget()
{
}

void FxCoarticulationWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* FxUnused(actor) )
{
	// Update the grid when an actor is loaded.
	_timeFormat = FxStudioOptions::GetTimelineGridFormat();
	FillGrid();
}

void FxCoarticulationWidget::OnPhonemeMappingChanged(FxWidget* FxUnused(sender), FxPhonemeMap* phonemeMap)
{	
	if( phonemeMap )
	{
	}
}

void FxCoarticulationWidget::OnAnimChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim )
{
	//@todo Implement this!  It should read the configuration from the animation
	//      user data and perform the same operations as OnGenericCoarticulationConfigChanged().
	_pAnim = anim;
}

void FxCoarticulationWidget::OnAnimPhonemeListChanged( FxWidget* FxUnused(sender), FxAnim* FxUnused(anim), FxPhonWordList* phonemeList )
{
	_pPhonemeList = phonemeList;
	if( phonemeList )
	{
	}
}

void FxCoarticulationWidget::OnRecalculateGrid( FxWidget* FxUnused(sender) )
{
	_timeFormat = FxStudioOptions::GetTimelineGridFormat();
	FillGrid();
}

void FxCoarticulationWidget::OnGenericCoarticulationConfigChanged( FxWidget* FxUnused(sender), FxCoarticulationConfig* FxUnused(config) )
{
	//@todo Implement this!
}

void FxCoarticulationWidget::OnSize( wxSizeEvent& FxUnused(event) )
{
	Layout();
}

void FxCoarticulationWidget::OnCellChange( wxGridEvent& event )
{
	if( event.GetCol() == 0 )
	{
		_config.phoneInfo[event.GetRow()].dominance = FxAtof(_parameterGrid->GetCellValue(event.GetRow(), event.GetCol()).mb_str(wxConvLibc));
	}
	else if( event.GetCol() == 1 )
	{
		_config.phoneInfo[event.GetRow()].anticipation = FxUnformatTime(_parameterGrid->GetCellValue(event.GetRow(), event.GetCol()), _timeFormat);
	}
}

void FxCoarticulationWidget::OnConfigChange( wxCommandEvent& FxUnused(event) )
{
	TransferDataFromWindow();
	wxString configName = _configName->GetValue();
	_config.SetName(FxString(configName.mb_str(wxConvLibc)).GetData());

	// Since the configuration has changed, we need to run through the phoneme
	// list changed code, because the coarticulator applies some rules while
	// phonemes are added into it.
	OnAnimPhonemeListChanged(this, _pAnim, _pPhonemeList);
}

// One-time initialization of the arrays.
void FxCoarticulationWidget::SetDefaultParameters( void )
{
	_config = FxCoarticulationConfig();
	FillGrid();
}

void FxCoarticulationWidget::FillGrid( void )
{
	TransferDataToWindow();
	FillConfigName();


	FxPhoneticAlphabet phoneticAlphabet = FxStudioOptions::GetPhoneticAlphabet();
	wxString rowLabel;
	for( FxSize i = 0; i < NUM_PHONS; ++i )
	{
		switch( phoneticAlphabet )
		{
		default:
		case PA_DEFAULT:
			rowLabel = FxGetPhonemeInfo(i).talkback;
			break;
		case PA_SAMPA:
			rowLabel = FxGetPhonemeInfo(i).sampa;
			break;
		case PA_IPA:
			rowLabel = FxGetPhonemeInfo(i).ipa;
			break;
		};

		_parameterGrid->SetRowLabelValue(i, rowLabel);
	}

	_parameterGrid->SetColLabelValue(0, _("Dominance"));
	_parameterGrid->SetColLabelValue(1, _("Anticipation"));

	for( FxSize i = 0; i < NUM_PHONS; ++i )
	{
		_parameterGrid->SetCellValue(wxString::Format(wxT("%0.3f"), _config.phoneInfo[i].dominance), i, 0);
		_parameterGrid->SetCellValue(FxFormatTime(_config.phoneInfo[i].anticipation, _timeFormat), i, 1);
	}
}

// Fills the name.
void FxCoarticulationWidget::FillConfigName( void )
{
	_configName->SetValue(wxString::FromAscii(_config.GetNameAsCstr()));
}

} // namespace Face

} // namespace OC3Ent
