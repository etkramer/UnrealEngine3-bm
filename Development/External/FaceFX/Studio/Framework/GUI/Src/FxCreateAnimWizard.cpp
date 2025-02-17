//------------------------------------------------------------------------------
// A wizard for creating new animations.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxCreateAnimWizard.h"

#include "FxStudioApp.h"
#include "FxSessionProxy.h"
#include "FxCGConfigManager.h"
#include "FxAudio.h"
#include "FxProgressDialog.h"
#include "FxConsole.h"
#include "FxFile.h"
#include "FxWidget.h"
#include "FxAudioFile.h"

#ifdef __UNREAL__
	#include "XWindow.h"
	#include "UnrealEd.h"
	#include "EngineSoundClasses.h"
#else
	#include "wx/valgen.h"
	#include "wx/textfile.h"
#endif

namespace OC3Ent
{
namespace Face
{

// Runs the animation creation wizard.  Returns FxTrue on success, or FxFalse on failure.
FxBool FxRunCreateAnimWizard(wxWindow* parent, const FxString& destGroup)
{
	wxBitmap wizardBmp;
	wxString bmpPath = FxStudioApp::GetBitmapPath(wxT("Wizard.bmp"));
	if( FxFileExists(bmpPath) )
	{
		wizardBmp.LoadFile(bmpPath, wxBITMAP_TYPE_BMP);
	}

	wxWizard* createAnimWizard = new wxWizard(parent, wxID_ANY, _("Create Animation Wizard"), wizardBmp);
	FxAnimWizardPageSelectType* typeSelectPage = new FxAnimWizardPageSelectType(createAnimWizard, NULL, NULL);
	FxAnimWizardPageSelectDirectory* dirSelectPage = new FxAnimWizardPageSelectDirectory(createAnimWizard, typeSelectPage, NULL);
	FxAnimWizardPageSelectAudio* audioSelectPage = new FxAnimWizardPageSelectAudio(createAnimWizard, typeSelectPage, NULL, typeSelectPage);
	typeSelectPage->SetNext(audioSelectPage);
	typeSelectPage->SetDirSelectPage(dirSelectPage);
#ifdef __UNREAL__
	FxAnimWizardPageSelectSoundNodeWave* soundNodeWaveSelectPage = new FxAnimWizardPageSelectSoundNodeWave(createAnimWizard, audioSelectPage, NULL, typeSelectPage);
	audioSelectPage->SetNext(soundNodeWaveSelectPage);
	FxAnimWizardPageSelectText* textSelectPage = new FxAnimWizardPageSelectText(createAnimWizard, soundNodeWaveSelectPage, NULL);
	soundNodeWaveSelectPage->SetNext(textSelectPage);
#else
	FxAnimWizardPageSelectText* textSelectPage = new FxAnimWizardPageSelectText(createAnimWizard, audioSelectPage, NULL);
	audioSelectPage->SetNext(textSelectPage);
#endif
	FxAnimWizardPageSelectOptions* optionsSelectPage = new FxAnimWizardPageSelectOptions(createAnimWizard, textSelectPage, NULL);
	textSelectPage->SetNext(optionsSelectPage);
	dirSelectPage->SetNext(optionsSelectPage);
	optionsSelectPage->SetTypeSelectPage(typeSelectPage);
	optionsSelectPage->SetDirSelectPage(dirSelectPage);
	FxAnimWizardPageSelectName* nameSelectPage = new FxAnimWizardPageSelectName(createAnimWizard, optionsSelectPage, NULL, typeSelectPage, destGroup);
	optionsSelectPage->SetNext(nameSelectPage);

	createAnimWizard->GetPageAreaSizer()->Add(typeSelectPage);

	FxBool result = createAnimWizard->RunWizard(typeSelectPage);
	if( result )
	{
		FxAnimWizardPageSelectType::FxAnimType type = typeSelectPage->GetAnimType();
		FxString animName = nameSelectPage->GetAnimName();

		if( type == FxAnimWizardPageSelectType::AT_AnalysisBased )
		{
			FxWString optionalText = textSelectPage->GetAnalysisText();

			FxString language = optionsSelectPage->GetLanguage();
			FxString gestureConfig = optionsSelectPage->GetGestureConfig();
			FxString coarticulationConfig = optionsSelectPage->GetCoarticulationConfig();
			FxBool   generateSpeechGestures = optionsSelectPage->GetGenerateSpeechGestures();

#ifdef __UNREAL__
			FxAudio* pAudio = NULL;
			FxString soundCuePath = audioSelectPage->GetAudioFilename();
			FxString soundNodeWaveName = soundNodeWaveSelectPage->GetSelectedSoundNodeWave();
			USoundCue* SoundCue = LoadObject<USoundCue>(NULL, ANSI_TO_TCHAR(soundCuePath.GetData()), NULL, LOAD_NoWarn, NULL);
			USoundNodeWave* SoundNodeWave = FindSoundNodeWave(SoundCue, soundNodeWaveName);
			if( SoundNodeWave )
			{
				pAudio = FxAudio::CreateFromSoundNodeWave(SoundNodeWave);
			}
			if( !FxSessionProxy::AnalyzeRawAudio(pAudio, optionalText, destGroup, animName, language, coarticulationConfig, gestureConfig, generateSpeechGestures, FxTrue) )
			{
				FxMessageBox(_("The audio analysis failed."), _("Cannot Create Animation"), wxOK);
			}
			else
			{
				// If the analysis was successful, set the SoundCue path and the
				// SoundNodeWave name.
				FxAnim* anim = NULL;
				if( FxSessionProxy::GetAnim(destGroup, animName, &anim) )
				{
					if( anim )
					{
						anim->SetSoundCuePath(soundCuePath);
						anim->SetSoundNodeWave(soundNodeWaveName);
						anim->SetSoundCuePointer(SoundCue);
					}
				}
			}
#else
			FxString audioFile = audioSelectPage->GetAudioFilename();
			if( !FxSessionProxy::AnalyzeFile(audioFile, optionalText, destGroup, animName, language, coarticulationConfig, gestureConfig, generateSpeechGestures, FxTrue) )
			{
				FxMessageBox(_("The audio analysis failed."), _("Cannot Create Animation"), wxOK);
			}
#endif
		}
		else if( type == FxAnimWizardPageSelectType::AT_NonAnalysisBased )
		{
			FxActor* pActor = NULL;
			
			if( FxSessionProxy::GetActor(&pActor) && pActor )
			{
				FxSize animGroupIndex = pActor->FindAnimGroup(destGroup.GetData());
				if( animGroupIndex != FxInvalidIndex )
				{
					FxAnimGroup& animGroup = pActor->GetAnimGroup(animGroupIndex);
					FxSize animIndex = animGroup.FindAnim(animName.GetData());
					if( animIndex != FxInvalidIndex )
					{
						// Remove the previous animation of the same name in the same group.
						// This is slightly different than the "Create Audio Animation" method
						// in that this method can be undone.
						FxString command = "anim -remove -group \"";
						command += destGroup;
						command += "\" -name \"";
						command += animName;
						command += "\"";
						FxVM::ExecuteCommand(command);
					}
				}
			}

			FxString command = "anim -add -group \"";
			command += destGroup;
			command += "\" -name \"";
			command += animName;
			command += "\"";
			if( CE_Success == FxVM::ExecuteCommand(command) )
			{
				FxAnim* anim = NULL;
				if( FxSessionProxy::GetAnim(destGroup, animName, &anim) && anim )
				{
					// If we should import audio on a silent animation, do so.
					if( typeSelectPage->GetImportAudio() )
					{
#ifdef __UNREAL__
						FxAudio* pAudio = NULL;
						FxString soundCuePath = audioSelectPage->GetAudioFilename();
						FxString soundNodeWaveName = soundNodeWaveSelectPage->GetSelectedSoundNodeWave();
						USoundCue* SoundCue = LoadObject<USoundCue>(NULL, ANSI_TO_TCHAR(soundCuePath.GetData()), NULL, LOAD_NoWarn, NULL);
						USoundNodeWave* SoundNodeWave = FindSoundNodeWave(SoundCue, soundNodeWaveName);
						if( SoundNodeWave )
						{
						pAudio = FxAudio::CreateFromSoundNodeWave(SoundNodeWave);
						}
						anim->SetSoundCuePath(soundCuePath);
						anim->SetSoundNodeWave(soundNodeWaveName);
						anim->SetSoundCuePointer(SoundCue);
#else
						FxString audioFilename = audioSelectPage->GetAudioFilename();
						FxAudio* pAudio = new FxAudio(audioFilename);
#endif
						if( pAudio )
						{
							pAudio->SetName(animName.GetData());
						}

						FxAnimUserData* animUserData = static_cast<FxAnimUserData*>(anim->GetUserData());
						if( animUserData )
						{
							animUserData->SetShouldContainAnalysisResults(FxFalse);
							animUserData->SetAudio(pAudio);
							if( pAudio )
							{
								animUserData->GetPhonemeWordListPointer()->ModifyPhonemeEndTime(0, pAudio->GetDuration());
							}
							else
							{
								animUserData->GetPhonemeWordListPointer()->ModifyPhonemeEndTime(0, 1.0f);
							}
							animUserData->SetIsSafeToUnload(FxFalse);
							FxSessionProxy::SetIsForcedDirty(FxTrue);
							FxSessionProxy::SetSelectedAnim(animName);
						}
						else
						{
							FxAssert(!"No user data attached to new animation.");
						}
						delete pAudio;
					}
					else
					{
						FxAnimUserData* animUserData = static_cast<FxAnimUserData*>(anim->GetUserData());
						if( animUserData )
						{
							animUserData->SetShouldContainAnalysisResults(FxFalse);
							animUserData->GetPhonemeWordListPointer()->ModifyPhonemeEndTime(0, 1.0f);
							animUserData->SetIsSafeToUnload(FxFalse);
							FxSessionProxy::SetIsForcedDirty(FxTrue);
							FxSessionProxy::SetSelectedAnim(animName);
						}
						else
						{
							FxAssert(!"No user data attached to new animation.");
						}
					}
				}
			}
		}
		else // type == AT_BatchAnalysis
		{
			FxString directory = dirSelectPage->GetDirectory();
			FxBool   recurse   = dirSelectPage->GetRecurse();
			FxBool   useText   = dirSelectPage->GetUseText();
			FxBool   overwrite = dirSelectPage->GetOverwrite();
			FxString group     = dirSelectPage->GetGroup();

			FxString language               = optionsSelectPage->GetLanguage();
			FxString gestureConfig          = optionsSelectPage->GetGestureConfig();
			FxString coarticulationConfig   = optionsSelectPage->GetCoarticulationConfig();
			FxBool   generateSpeechGestures = optionsSelectPage->GetGenerateSpeechGestures();

			if( !FxSessionProxy::AnalyzeDirectory(directory, group, language,
				coarticulationConfig, gestureConfig, generateSpeechGestures,
				overwrite, recurse, !useText) )
			{
				FxMessageBox(_("The audio analysis failed."), _("Cannot Batch Process"), wxOK);
			}
		}
	}

	createAnimWizard->Destroy();
	return result;
}


//------------------------------------------------------------------------------
// FxAnimWizardPageSelectType
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxAnimWizardPageSelectType, wxWizardPage)
	EVT_RADIOBOX(ControlID_CreateAnimWizardSelectTypePageAnimTypeRadio, FxAnimWizardPageSelectType::OnAnimTypeChange)
END_EVENT_TABLE()

FxAnimWizardPageSelectType::FxAnimWizardPageSelectType(wxWizard* parent, wxWizardPage* prev, wxWizardPage* next)
	: wxWizardPage(parent)
{
	_prevPage = prev;
	_nextPage = next;


	wxString batchAnalysis(_("Batch analysis"));
#ifdef FX_NO_ANALYZE_COMMAND
	batchAnalysis += _(" is disabled in ");
	#if defined(MOD_DEVELOPER_VERSION)
		batchAnalysis += _("the mod developer version");
	#elif defined(EVALUATION_VERSION)
		batchAnalysis += _("evaluation versions");
	#else
		batchAnalysis += _("this version");
	#endif
#endif // FX_NO_ANALYZE_COMMAND
	wxString choices[] = {_("Automatically generate speech curves, and optionally, gesture curves"), _("Do not generate curves"), batchAnalysis};
	_animTypeRadio = new wxRadioBox(this, ControlID_CreateAnimWizardSelectTypePageAnimTypeRadio, _("Analysis Type"), wxDefaultPosition, wxDefaultSize,
#ifdef __UNREAL__
		2,
#else
		3,
#endif // __UNREAL__
		choices, 0, wxRA_VERTICAL);
	_animTypeRadio->SetSelection(0);

#ifndef __UNREAL__
	#ifdef FX_NO_ANALYZE_COMMAND
		// Disable batch analysis in the evaluation version.
		_animTypeRadio->Enable(2, false);
	#endif // FX_NO_ANALYZE_COMMAND
#endif // __UNREAL__

#ifdef __UNREAL__
	_importAudioCheck = new wxCheckBox(this, ControlID_CreateAnimWizardSelectTypePageImportAudioCheck, _("Use SoundCue"));
#else
	_importAudioCheck = new wxCheckBox(this, ControlID_CreateAnimWizardSelectTypePageImportAudioCheck, _("Import Audio"));
#endif // __UNREAL__
	_importAudioCheck->SetValue(FxFalse);
	_importAudioCheck->Enable(FxFalse);

	SetSizer(new wxBoxSizer(wxVERTICAL));
	GetSizer()->Add(
		new wxStaticText(this, wxID_ANY, 
		_("This wizard will create a new animation.\nFirst, specify if you want FaceFX to generate speech curves automatically.")),
		0, wxALL|wxALIGN_LEFT, 5);
	GetSizer()->AddStretchSpacer(1);
	GetSizer()->Add(_animTypeRadio, 0, wxALL|wxEXPAND|wxALIGN_CENTRE_HORIZONTAL, 5);
	GetSizer()->Add(_importAudioCheck, 0, wxALL|wxALIGN_LEFT, 5);
	GetSizer()->AddStretchSpacer(1);
}

void FxAnimWizardPageSelectType::SetPrev(wxWizardPage* prev)
{
	_prevPage = prev;
}

void FxAnimWizardPageSelectType::SetNext(wxWizardPage* next)
{
	_nextPage = next;
}

void FxAnimWizardPageSelectType::SetDirSelectPage(FxAnimWizardPageSelectDirectory* dirSelectPage)
{
	_dirSelectPage = dirSelectPage;
}

wxWizardPage* FxAnimWizardPageSelectType::GetPrev() const
{
	return _prevPage;
}

wxWizardPage* FxAnimWizardPageSelectType::GetNext() const
{
	FxAnimType type = GetAnimType();
	if( type == AT_AnalysisBased )
	{
		// If we're analysis based, go to the audio select screen.
		return _nextPage;
	}
	else if( type == AT_NonAnalysisBased )
	{
		if( GetImportAudio() )
		{
			// If we want to import audio, go to the audio select screen.
			return _nextPage;
		}
		else
		{
			// If we don't want to import audio, go to the name screen.
#ifdef __UNREAL__
			return _nextPage->GetNext()->GetNext();
#else
			return _nextPage->GetNext();
#endif
		}
	}
	else if( type == AT_BatchAnalysis )
	{
		return _dirSelectPage;
	}
	else
	{
		return NULL;
	}
}

FxAnimWizardPageSelectType::FxAnimType FxAnimWizardPageSelectType::GetAnimType() const
{
	return static_cast<FxAnimType>(_animTypeRadio->GetSelection());
}

FxBool FxAnimWizardPageSelectType::GetImportAudio() const
{
	return _importAudioCheck->GetValue();
}

void FxAnimWizardPageSelectType::OnAnimTypeChange(wxCommandEvent&)
{
	_importAudioCheck->Enable(_animTypeRadio->GetSelection() == 1);
}

//------------------------------------------------------------------------------
// FxAnimWizardPageSelectDirectory
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxAnimWizardPageSelectDirectory, wxWizardPage)
	EVT_BUTTON(ControlID_CreateAnimWizardSelectDirectoryPageBrowseButton, FxAnimWizardPageSelectDirectory::OnBrowseButton)
	EVT_RADIOBOX(ControlID_CreateAnimWizardSelectDirectoryPageGroupMethodRadio, FxAnimWizardPageSelectDirectory::OnGroupMethodChange)
	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, FxAnimWizardPageSelectDirectory::OnPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, FxAnimWizardPageSelectDirectory::OnPageChanging)
END_EVENT_TABLE()

FxAnimWizardPageSelectDirectory::FxAnimWizardPageSelectDirectory(wxWizard* parent, wxWizardPage* prev, wxWizardPage* next)
	: wxWizardPage(parent)
	, _recurse(FxTrue)
	, _useText(FxTrue)
	, _overwrite(FxTrue)
	, _groupMethod(0)
{
	SetPrev(prev);
	SetNext(next);

	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	FxInt32 numChoices = 0;
	wxString* groupChoices = NULL;
	if( pActor )
	{
		numChoices = pActor->GetNumAnimGroups();
		groupChoices = new wxString[numChoices];
		for( FxInt32 i = 0; i < numChoices; ++i )
		{
			groupChoices[i] = wxString::FromAscii(
				pActor->GetAnimGroup(i).GetName().GetAsCstr());
		}
	}

	wxStaticText* pageLabel = new wxStaticText(this, wxID_ANY, _("Select the directory to analyze."));
	wxStaticText* label = new wxStaticText(this, wxID_ANY, _("Directory:"));
	wxTextCtrl* directoryCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(275,20), 0, wxGenericValidator(&_directory));
	wxButton* browseButton = new wxButton(this, ControlID_CreateAnimWizardSelectDirectoryPageBrowseButton, _("Browse..."));
	wxCheckBox* recurseCheck = new wxCheckBox(this, wxID_ANY, _("Recurse subdirectories"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_recurse));
	wxCheckBox* useTextCheck = new wxCheckBox(this, wxID_ANY, _("Use text if available"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_useText));
	wxCheckBox* overwriteCheck = new wxCheckBox(this, wxID_ANY, _("Overwrite existing animations"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_overwrite));
	wxString groupMethodChoices[] = {_("Create animation groups from path name"), _("Place all animations in selected animation group")};
	_groupMethodRadio = new wxRadioBox(this, ControlID_CreateAnimWizardSelectDirectoryPageGroupMethodRadio, _("Destination Anim Group"), wxDefaultPosition, wxDefaultSize,
		2, groupMethodChoices, 0, wxRA_VERTICAL);
	_groupCombo = new wxComboBox(this, ControlID_CreateAnimWizardSelectDirectoryPageGroupChoice, _(""), wxDefaultPosition, wxSize(280,20), numChoices, groupChoices);
	delete[] groupChoices; groupChoices = NULL;
	_groupMethodRadio->SetSelection(0);

	FxString selAnimGroup;
	if( FxSessionProxy::GetSelectedAnimGroup(selAnimGroup) )
	{
		_groupCombo->SetValue(wxString::FromAscii(selAnimGroup.GetData()));
	}

	SetSizer(new wxBoxSizer(wxVERTICAL));
	GetSizer()->Add(pageLabel, 0, wxALL, 5);
	GetSizer()->AddStretchSpacer(1);
	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(label, 0, wxALL|wxALIGN_RIGHT, 5);
	row->Add(directoryCtrl, 1, wxALL|wxALIGN_RIGHT, 5);
	GetSizer()->Add(row, 0, wxALIGN_CENTRE_VERTICAL|wxEXPAND, 5);
	GetSizer()->Add(browseButton, 0, wxALL|wxALIGN_CENTRE_VERTICAL|wxALIGN_RIGHT, 5);
	GetSizer()->Add(recurseCheck, 0, wxALL, 5);
	GetSizer()->Add(overwriteCheck, 0, wxALL, 5);
	GetSizer()->Add(useTextCheck, 0, wxALL, 5);
	GetSizer()->Add(_groupMethodRadio, 0, wxALL, 5);
	row = new wxBoxSizer(wxHORIZONTAL);
	_groupLabel = new wxStaticText(this, wxID_ANY, _("Animation Group:"));
	row->Add(_groupLabel, 0, wxALL, 5);
	row->Add(_groupCombo, 0, wxALL, 5);
	GetSizer()->Add(row);
	GetSizer()->AddStretchSpacer(1);

	// Set the correct combo state.
	wxCommandEvent tempEvent;
	OnGroupMethodChange(tempEvent);
}

void FxAnimWizardPageSelectDirectory::SetPrev(wxWizardPage* prev)
{
	_prevPage = prev;
}

void FxAnimWizardPageSelectDirectory::SetNext(wxWizardPage* next)
{
	_nextPage = next;
}

wxWizardPage* FxAnimWizardPageSelectDirectory::GetPrev() const
{
	return _prevPage;
}

wxWizardPage* FxAnimWizardPageSelectDirectory::GetNext() const
{
	return _nextPage;
}

FxString FxAnimWizardPageSelectDirectory::GetDirectory() const
{
	return FxString(_directory.mb_str(wxConvLibc));
}

FxBool FxAnimWizardPageSelectDirectory::GetRecurse() const
{
	return _recurse;
}

FxBool FxAnimWizardPageSelectDirectory::GetUseText() const
{
	return _useText;
}

FxBool FxAnimWizardPageSelectDirectory::GetOverwrite() const
{
	return _overwrite;
}

FxString FxAnimWizardPageSelectDirectory::GetGroup() const
{
	if( _groupMethodRadio->GetSelection() == 1 )
	{
		return FxString(_groupCombo->GetValue().mb_str(wxConvLibc));
	}
	else
	{
		return FxString("");
	}
}

void FxAnimWizardPageSelectDirectory::OnBrowseButton(wxCommandEvent&)
{	
	TransferDataFromWindow();
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	wxString dir = ::wxDirSelector(_("Choose a folder"), _directory);
	if( !dir.empty() )
	{
		_directory = dir;
		TransferDataToWindow();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxAnimWizardPageSelectDirectory::OnGroupMethodChange(wxCommandEvent&)
{
	FxBool enabled = _groupMethodRadio->GetSelection() == 1;
	_groupCombo->Enable(enabled);
	_groupLabel->Enable(enabled);
}

void FxAnimWizardPageSelectDirectory::OnPageChanged(wxWizardEvent&)
{
}

void FxAnimWizardPageSelectDirectory::OnPageChanging(wxWizardEvent& event)
{
	TransferDataFromWindow();
	if( event.GetDirection() && _directory.IsEmpty() )
	{
		FxMessageBox(_("You must select a folder to analyze before proceeding."), _("Error"));
		event.Veto();
	}
}

//------------------------------------------------------------------------------
// FxAnimWizardPageSelectAudio
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxAnimWizardPageSelectAudio, wxWizardPage)
	EVT_BUTTON(ControlID_CreateAnimWizardSelectAudioPageBrowseButton, FxAnimWizardPageSelectAudio::OnBrowseButton)
	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, FxAnimWizardPageSelectAudio::OnPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, FxAnimWizardPageSelectAudio::OnPageChanging)
END_EVENT_TABLE()

FxAnimWizardPageSelectAudio::FxAnimWizardPageSelectAudio(wxWizard* parent, wxWizardPage* prev, wxWizardPage* next, FxAnimWizardPageSelectType* typeSelectPage)
	: wxWizardPage(parent)
{
	_prevPage = prev;
	_nextPage = next;
	_typeSelectPage = typeSelectPage;

#ifdef __UNREAL__
	_audioFilename = _("None");

	wxStaticText* pageLabel = new wxStaticText(this, wxID_ANY, _("Next, select the SoundCue to use."));
	wxStaticText* label = new wxStaticText(this, wxID_ANY, _("SoundCue:"));
	wxTextCtrl* audioFilenameCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(275,20), 0, wxGenericValidator(&_audioFilename));
	wxButton* browseButton = new wxButton(this, ControlID_CreateAnimWizardSelectAudioPageBrowseButton, _("Use Current Selection In Browser"));
#else
	wxStaticText* pageLabel = new wxStaticText(this, wxID_ANY, _("Next, select the audio file to import."));
	wxStaticText* label = new wxStaticText(this, wxID_ANY, _("Audio filename:"));
	wxTextCtrl* audioFilenameCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(275,20), 0, wxGenericValidator(&_audioFilename));
	wxButton* browseButton = new wxButton(this, ControlID_CreateAnimWizardSelectAudioPageBrowseButton, _("Browse..."));
#endif

	SetSizer(new wxBoxSizer(wxVERTICAL));
	GetSizer()->Add(pageLabel, 0, wxALL, 5);
	GetSizer()->AddStretchSpacer(1);
	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(label, 0, wxALL|wxALIGN_RIGHT, 5);
	row->Add(audioFilenameCtrl, 1, wxALL|wxALIGN_RIGHT, 5);
	GetSizer()->Add(row, 0, wxALIGN_CENTRE_VERTICAL|wxEXPAND, 5);
	GetSizer()->Add(browseButton, 0, wxALL|wxALIGN_CENTRE_VERTICAL|wxALIGN_RIGHT, 5);
	GetSizer()->AddStretchSpacer(1);
}

void FxAnimWizardPageSelectAudio::SetPrev(wxWizardPage* prev)
{
	_prevPage = prev;
}

void FxAnimWizardPageSelectAudio::SetNext(wxWizardPage* next)
{
	_nextPage = next;
}

wxWizardPage* FxAnimWizardPageSelectAudio::GetPrev() const
{
	return _prevPage;
}

wxWizardPage* FxAnimWizardPageSelectAudio::GetNext() const
{
	if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_AnalysisBased )
	{
		return _nextPage;
	}
	else if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_NonAnalysisBased )
	{
#ifdef __UNREAL__
		return _nextPage;
#else
		return _nextPage->GetNext()->GetNext();
#endif
	}
	else
	{
		return NULL;
	}
}

FxString FxAnimWizardPageSelectAudio::GetAudioFilename() const
{
	return FxString(_audioFilename.mb_str(wxConvLibc));
}

void FxAnimWizardPageSelectAudio::OnBrowseButton(wxCommandEvent&)
{
#ifdef __UNREAL__
	if( GEditor->GetSelectedObjects()->GetTop(USoundCue::StaticClass()) )
	{
		_audioFilename = *GEditor->GetSelectedObjects()->GetTop(USoundCue::StaticClass())->GetPathName();
		TransferDataToWindow();
	}
	else
	{
		_audioFilename = _("None");
		TransferDataToWindow();
	}
#else
	FxArray<FxString> exts  = FxAudioFile::GetSupportedExtensions();
	FxArray<FxString> descs = FxAudioFile::GetDescriptions();
	FxSize numExts = exts.Length();
	FxAssert(numExts == descs.Length());
	wxString fileTypes("", wxConvLibc);
	// Make the default choice for all supported audio file types.
	if( numExts > 1 )
	{
		fileTypes += wxString("All Supported Audio Files (", wxConvLibc);
		wxString allSupported;
		for( FxSize i = 0; i < numExts; ++i )
		{
			allSupported += wxString("*.", wxConvLibc);
			allSupported += wxString(exts[i].GetData(), wxConvLibc);
			// Don't append a semicolon on the last one.
			if( i != numExts - 1 )
			{
				allSupported += wxString(";", wxConvLibc);
			}
		}
		fileTypes += allSupported;
		fileTypes += wxString(")|", wxConvLibc);
		fileTypes += allSupported;
		fileTypes += wxString("|", wxConvLibc);
	}
	for( FxSize i = 0; i < numExts; ++i )
	{
		fileTypes += wxString(descs[i].GetData(), wxConvLibc);
		fileTypes += wxString(" (*.", wxConvLibc);
		fileTypes += wxString(exts[i].GetData(), wxConvLibc);
		fileTypes += wxString(")|*.", wxConvLibc);
		fileTypes += wxString(exts[i].GetData(), wxConvLibc);
		fileTypes += wxString("|", wxConvLibc);
	}
	fileTypes += wxString("All Files (*.*)|*.*", wxConvLibc);
	wxFileDialog fileDlg(this, _("Select an audio file"), wxEmptyString, wxEmptyString, fileTypes, wxFD_OPEN);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( fileDlg.ShowModal() == wxID_OK )
	{
		_audioFilename = fileDlg.GetPath();
		TransferDataToWindow();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
#endif
}

void FxAnimWizardPageSelectAudio::OnPageChanged(wxWizardEvent&)
{
#ifdef __UNREAL__
	if( GEditor->GetSelectedObjects()->GetTop(USoundCue::StaticClass()) )
	{
		_audioFilename = *GEditor->GetSelectedObjects()->GetTop(USoundCue::StaticClass())->GetPathName();
		TransferDataToWindow();
	}
#endif
}

void FxAnimWizardPageSelectAudio::OnPageChanging(wxWizardEvent& event)
{
	TransferDataFromWindow();
#ifdef __UNREAL__
	if( event.GetDirection() && _audioFilename.IsEmpty() )
	{
		FxMessageBox(_("You must select a SoundCue to use before proceeding."), _("Error"));
		event.Veto();
	}
	else
	{
		USoundCue* SoundCue = LoadObject<USoundCue>(NULL, ANSI_TO_TCHAR(_audioFilename.mb_str(wxConvLibc)), NULL, LOAD_NoWarn, NULL);
		if( event.GetDirection() && !SoundCue )
		{
			FxMessageBox(_("You must select a SoundCue to use before proceeding."), _("Error"));
			event.Veto();
		}
	}
#else
	if( event.GetDirection() && (_audioFilename.IsEmpty() || !FxFileExists(_audioFilename)) )
	{
		FxMessageBox(_("You must select an audio file to import before proceeding."), _("Error"));
		event.Veto();
	}
#endif
}

#ifdef __UNREAL__

//------------------------------------------------------------------------------
// FxAnimWizardPageSelectSoundNodeWave
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxAnimWizardPageSelectSoundNodeWave, wxWizardPage)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, FxAnimWizardPageSelectSoundNodeWave::OnPageChanging)
	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, FxAnimWizardPageSelectSoundNodeWave::OnPageChanged)
END_EVENT_TABLE()

FxAnimWizardPageSelectSoundNodeWave::FxAnimWizardPageSelectSoundNodeWave(wxWizard* parent, wxWizardPage* prev, wxWizardPage* next, FxAnimWizardPageSelectType* typeSelectPage)
	: wxWizardPage(parent)
{
	_prevPage = prev;
	_nextPage = next;
	_typeSelectPage = typeSelectPage;

	wxStaticText* pageLabel = new wxStaticText(this, wxID_ANY, _("Next, specify the SoundNodeWave to analyze with."));
	wxStaticText* listBoxLabel = new wxStaticText(this, wxID_ANY, _("SoundNodeWaves:"));
	_soundNodeWaveListBox = new wxListBox(this, ControlID_CreateAnimWizardSelectSoundNodeWavePageSoundNodeWaveListBox, wxDefaultPosition, wxSize(320,100));
	
	SetSizer(new wxBoxSizer(wxVERTICAL));
	GetSizer()->Add(pageLabel, 0, wxALL, 5);
	GetSizer()->AddStretchSpacer(1);
	GetSizer()->Add(listBoxLabel, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL, 5);
	GetSizer()->Add(_soundNodeWaveListBox, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL|wxEXPAND, 5);
	GetSizer()->AddStretchSpacer(1);
}

void FxAnimWizardPageSelectSoundNodeWave::SetPrev(wxWizardPage* prev)
{
	_prevPage = prev;
}

void FxAnimWizardPageSelectSoundNodeWave::SetNext(wxWizardPage* next)
{
	_nextPage = next;
}

wxWizardPage* FxAnimWizardPageSelectSoundNodeWave::GetPrev() const
{
	return _prevPage;
}

wxWizardPage* FxAnimWizardPageSelectSoundNodeWave::GetNext() const
{
	if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_AnalysisBased )
	{
		return _nextPage;
	}
	else if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_NonAnalysisBased )
	{
		return _nextPage->GetNext()->GetNext();
	}
	else
	{
		return NULL;
	}
}

FxString FxAnimWizardPageSelectSoundNodeWave::GetSelectedSoundNodeWave() const
{
	return FxString(_selectedSoundNodeWave.mb_str(wxConvLibc));
}

void FxAnimWizardPageSelectSoundNodeWave::OnPageChanging(wxWizardEvent& event)
{
	if( _soundNodeWaveListBox )
	{
		if( event.GetDirection() && -1 == _soundNodeWaveListBox->GetSelection() )
		{
			FxMessageBox(_("You must select a SoundNodeWave to use before proceeding."), _("Error"));
			event.Veto();
		}
		else
		{
			_selectedSoundNodeWave = _soundNodeWaveListBox->GetStringSelection();
		}
	}
}

void FxAnimWizardPageSelectSoundNodeWave::OnPageChanged(wxWizardEvent&)
{
	if( _soundNodeWaveListBox )
	{
		_soundNodeWaveListBox->Clear();
		FxAnimWizardPageSelectAudio* audioPage = static_cast<FxAnimWizardPageSelectAudio*>(GetPrev());
		FxString soundCuePath = audioPage->GetAudioFilename();

		USoundCue* SoundCue = LoadObject<USoundCue>(NULL, ANSI_TO_TCHAR(soundCuePath.GetData()), NULL, LOAD_NoWarn, NULL);
		FxArray<wxString> SoundNodeWaves = FindAllSoundNodeWaves(SoundCue);
		for( FxSize i = 0; i < SoundNodeWaves.Length(); ++i )
		{
			_soundNodeWaveListBox->AppendString(SoundNodeWaves[i]);
		}
		
		if( _soundNodeWaveListBox->GetCount() > 0 )
		{
			_soundNodeWaveListBox->SetSelection(0);
		}
	}
}

#endif

//------------------------------------------------------------------------------
// FxAnimWizardPageSelectText
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxAnimWizardPageSelectText, wxWizardPage)
	EVT_BUTTON(ControlID_CreateAnimWizardSelectAudioPageBrowseButton, FxAnimWizardPageSelectText::OnBrowseButton)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, FxAnimWizardPageSelectText::OnPageChanging)
	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, FxAnimWizardPageSelectText::OnPageChanged)
END_EVENT_TABLE()

FxAnimWizardPageSelectText::FxAnimWizardPageSelectText(wxWizard* parent, wxWizardPage* prev, wxWizardPage* next)
	: wxWizardPage(parent)
{
	_prevPage = prev;
	_nextPage = next;

	wxStaticText* pageLabel = new wxStaticText(this, wxID_ANY, _("Specify the text to analyze with, or leave the box blank for textless analysis."));
	wxStaticText* label = new wxStaticText(this, wxID_ANY, _("Analysis text:"));
	wxTextCtrl* analysisTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(320,100), wxTE_MULTILINE, wxGenericValidator(&_analysisText));
	analysisTextCtrl->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, FxStudioOptions::GetUnicodeFontName(), wxFONTENCODING_SYSTEM));
	wxButton* browseButton = new wxButton(this, ControlID_CreateAnimWizardSelectAudioPageBrowseButton, _("Browse..."));

	SetSizer(new wxBoxSizer(wxVERTICAL));
	GetSizer()->Add(pageLabel, 0, wxALL, 5);
	GetSizer()->AddStretchSpacer(1);
	GetSizer()->Add(label, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL, 5);
	GetSizer()->Add(analysisTextCtrl, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL|wxEXPAND, 5);
	GetSizer()->Add(browseButton, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTRE_VERTICAL, 5);
	GetSizer()->AddStretchSpacer(1);
}

void FxAnimWizardPageSelectText::SetPrev(wxWizardPage* prev)
{
	_prevPage = prev;
}

void FxAnimWizardPageSelectText::SetNext(wxWizardPage* next)
{
	_nextPage = next;
}

wxWizardPage* FxAnimWizardPageSelectText::GetPrev() const
{
	return _prevPage;
}

wxWizardPage* FxAnimWizardPageSelectText::GetNext() const
{
	return _nextPage;
}

FxWString FxAnimWizardPageSelectText::GetAnalysisText() const
{
	return FxWString(_analysisText.GetData());
}

void FxAnimWizardPageSelectText::OnBrowseButton(wxCommandEvent&)
{
	wxFileDialog fileDlg(this, _("Select a text file"), wxEmptyString, wxEmptyString, _("Text files (*.txt)|*.txt|All Files (*.*)|*.*"), wxFD_OPEN);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	if( fileDlg.ShowModal() == wxID_OK )
	{
		_analysisText = FxLoadUnicodeTextFile(FxString(fileDlg.GetPath().mb_str(wxConvLibc)));
		TransferDataToWindow();
	}
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
}

void FxAnimWizardPageSelectText::OnPageChanging(wxWizardEvent&)
{
	TransferDataFromWindow();
}

void FxAnimWizardPageSelectText::OnPageChanged(wxWizardEvent& event)
{
	if( event.GetDirection() )
	{
#ifdef __UNREAL__
		FxAnimWizardPageSelectSoundNodeWave* soundNodeWaveSelectPage = static_cast<FxAnimWizardPageSelectSoundNodeWave*>(GetPrev());
		FxAnimWizardPageSelectAudio* audioSelectPage = static_cast<FxAnimWizardPageSelectAudio*>(soundNodeWaveSelectPage->GetPrev());
		FxString soundCuePath = audioSelectPage->GetAudioFilename();
		FxString soundNodeWaveName = soundNodeWaveSelectPage->GetSelectedSoundNodeWave();
		USoundCue* SoundCue = LoadObject<USoundCue>(NULL, ANSI_TO_TCHAR(soundCuePath.GetData()), NULL, LOAD_NoWarn, NULL);
		USoundNodeWave* SoundNodeWave = FindSoundNodeWave(SoundCue, soundNodeWaveName);
		if( SoundNodeWave )
		{
			if( SoundNodeWave->SpokenText.Len() > 0 )
			{
				wxString testString(*SoundNodeWave->SpokenText);
				// Strip newlines, carriage returns and tabs.
				wxString correctedText;
				for( FxSize i = 0; i < testString.Length(); ++i )
				{
					if( testString[i] == wxT('\r') || testString[i] == wxT('\n') || testString[i] == wxT('\t') )
					{
						correctedText.Append(wxT(' '));
					}
					else
					{
						correctedText.Append(testString[i]);
					}
				}

				_analysisText = correctedText;
				TransferDataToWindow();
			}
		}
#else
		FxAnimWizardPageSelectAudio* audioPage = static_cast<FxAnimWizardPageSelectAudio*>(GetPrev());
		FxString audioFilename = audioPage->GetAudioFilename();
		_analysisText = FxLoadUnicodeTextFile(audioFilename.BeforeLast('.') + ".txt");
		TransferDataToWindow();
#endif
	}
}

//------------------------------------------------------------------------------
// FxAnimWizardPageSelectOptions
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxAnimWizardPageSelectOptions, wxWizardPage)
	EVT_CHOICE(ControlID_CreateAnimWizardSelectOptionsPageLanguageChoice, FxAnimWizardPageSelectOptions::OnLanguageChoiceChange)
	EVT_BUTTON(ControlID_CreateAnimWizardSelectOptionsPageSelectGestureConfigButton, FxAnimWizardPageSelectOptions::OnSelectConfigButtons)
	EVT_BUTTON(ControlID_CreateAnimWizardSelectOptionsPageSelectCoarticulationConfigButton, FxAnimWizardPageSelectOptions::OnSelectConfigButtons)
END_EVENT_TABLE()

FxAnimWizardPageSelectOptions::FxAnimWizardPageSelectOptions(wxWizard* parent, wxWizardPage* prev, wxWizardPage* next)
	: wxWizardPage(parent)
{
	FxArray<FxLanguageInfo> languages;
	FxSessionProxy::QueryLanguageInfo(languages);
	for( FxSize i = 0; i < languages.Length(); ++i )
	{
		if( languages[i].isDefault )
		{
			_defaultLanguage = languages[i].languageName;
			break;
		}
	}

	_prevPage = prev;
	_nextPage = next;
	_language = _defaultLanguage;
	_gestureConfig = FxCGConfigManager::GetDefaultGestureConfigName().GetAsString();
	_coarticulationConfig = FxCGConfigManager::GetDefaultGenericCoarticulationConfigName().GetAsString();
	_generateSpeechGestures = FxTrue;

	FxInt32 numChoices = languages.Length();
	wxString* choices;
	choices = new wxString[numChoices];
	for( FxSize i = 0; i < languages.Length(); ++i )
	{
		choices[i] = wxString::FromAscii(languages[i].languageName.GetData());
	}

	_languageChoice = new wxChoice(this, ControlID_CreateAnimWizardSelectOptionsPageLanguageChoice, wxDefaultPosition, wxDefaultSize, numChoices, choices);
	_languageChoice->SetStringSelection(wxString::FromAscii(_defaultLanguage.GetData()));
	_gestureConfigLabel = new wxStaticText(this, wxID_ANY, _("Gesture Config: ") + wxString::FromAscii(*_gestureConfig), wxDefaultPosition, wxSize(100,20), wxST_NO_AUTORESIZE|wxSIMPLE_BORDER);
	_coarticulationConfigLabel = new wxStaticText(this, wxID_ANY, _("Coarticulation Config: ") + wxString::FromAscii(*_coarticulationConfig), wxDefaultPosition, wxSize(100,20), wxST_NO_AUTORESIZE|wxSIMPLE_BORDER);
	_speechGesturesCheck = new wxCheckBox(this, ControlID_CreateAnimWizardSelectOptionsPageSpeechGesturesCheck, _("Generate Speech Gestures"), wxDefaultPosition, wxDefaultSize, 0, wxGenericValidator(&_generateSpeechGestures));
	delete[] choices;

	SetSizer(new wxBoxSizer(wxVERTICAL));
	GetSizer()->Add(new wxStaticText(this, wxID_ANY, _("Select the language of the audio (and text).\nSelect whether you want speech gestures to be generated.\n You may also change the gesture and coarticulation configurations, if desired.")), 0, wxALL, 5);
	GetSizer()->AddStretchSpacer(1);
	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(new wxStaticText(this, wxID_ANY, _("Language:")), 0, wxALL, 5);
	row->Add(_languageChoice, 0, wxALL, 5);
	GetSizer()->Add(row);
	GetSizer()->Add(_speechGesturesCheck, 0, wxALL, 5);
	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(_gestureConfigLabel, 1, wxALL, 5);
	row->Add(new wxButton(this, ControlID_CreateAnimWizardSelectOptionsPageSelectGestureConfigButton, _("Change Gesture Config...")), 1, wxALL, 5);
	GetSizer()->Add(row, 0, wxEXPAND, 0);
	row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(_coarticulationConfigLabel, 1, wxALL, 5);
	row->Add(new wxButton(this, ControlID_CreateAnimWizardSelectOptionsPageSelectCoarticulationConfigButton, _("Change Coarticulation Config...")), 1, wxALL, 5);
	GetSizer()->Add(row, 0, wxEXPAND, 0);
	GetSizer()->AddStretchSpacer(1);

	TransferDataToWindow();
}

void FxAnimWizardPageSelectOptions::SetPrev(wxWizardPage* prev)
{
	_prevPage = prev;
}

void FxAnimWizardPageSelectOptions::SetNext(wxWizardPage* next)
{
	_nextPage = next;
}

wxWizardPage* FxAnimWizardPageSelectOptions::GetPrev() const
{
	if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_BatchAnalysis )
	{
		return _dirSelectPage;
	}
	return _prevPage;
}

wxWizardPage* FxAnimWizardPageSelectOptions::GetNext() const
{
	if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_BatchAnalysis )
	{
		return NULL;
	}
	return _nextPage;
}

void FxAnimWizardPageSelectOptions::SetDirSelectPage(FxAnimWizardPageSelectDirectory* dirSelectPage)
{
	_dirSelectPage = dirSelectPage;
}

void FxAnimWizardPageSelectOptions::SetTypeSelectPage(FxAnimWizardPageSelectType* typeSelectPage)
{
	_typeSelectPage = typeSelectPage;
}

FxString FxAnimWizardPageSelectOptions::GetLanguage()
{
	return _language;
}

FxString FxAnimWizardPageSelectOptions::GetGestureConfig()
{
	return _gestureConfig;
}

FxString FxAnimWizardPageSelectOptions::GetCoarticulationConfig()
{
	return _coarticulationConfig;
}

FxBool FxAnimWizardPageSelectOptions::GetGenerateSpeechGestures()
{
	return _generateSpeechGestures;
}

void FxAnimWizardPageSelectOptions::OnLanguageChoiceChange(wxCommandEvent&)
{
	_language = _languageChoice->GetStringSelection().mb_str(wxConvLibc);
}

void FxAnimWizardPageSelectOptions::OnSelectConfigButtons(wxCommandEvent& event)
{
	FxCGConfigChooser::FxConfigChooserMode mode = event.GetId() == ControlID_CreateAnimWizardSelectOptionsPageSelectGestureConfigButton ? FxCGConfigChooser::CCM_Gesture : FxCGConfigChooser::CCM_GenericCoarticulation;
	wxString message = wxString(_("Select the ")) + wxString(event.GetId() == ControlID_CreateAnimWizardSelectOptionsPageSelectGestureConfigButton ? _("gesture") : _("coarticulation")) + wxString(_(" configuration for the animation"));

	FxCGConfigChooser configChooser(mode, message);
	FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
	configChooser.ShowModal();
	FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();

	if( event.GetId() == ControlID_CreateAnimWizardSelectOptionsPageSelectGestureConfigButton )
	{
		_gestureConfig = FxString(configChooser.ChosenConfigName.mb_str(wxConvLibc));
	}
	else // event.m_id == ControlID_CreateAnimWizardSelectOptionsPageSelectCoarticulationConfigButton.
	{
		_coarticulationConfig = FxString(configChooser.ChosenConfigName.mb_str(wxConvLibc));
	} 

	_gestureConfigLabel->SetLabel(_("Gesture Config: ") + wxString::FromAscii(*_gestureConfig));
	_coarticulationConfigLabel->SetLabel(_("Coarticulation Config: ") + wxString::FromAscii(*_coarticulationConfig));
}

//------------------------------------------------------------------------------
// FxAnimWizardPageSelectName
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(FxAnimWizardPageSelectName, wxWizardPage)
	EVT_WIZARD_PAGE_CHANGED(wxID_ANY, FxAnimWizardPageSelectName::OnPageChanged)
	EVT_WIZARD_PAGE_CHANGING(wxID_ANY, FxAnimWizardPageSelectName::OnPageChanging)
END_EVENT_TABLE()

FxAnimWizardPageSelectName::FxAnimWizardPageSelectName(wxWizard* parent, wxWizardPage* prev, wxWizardPage* next, FxAnimWizardPageSelectType* typeSelectPage, const FxString& animGroup)
	: wxWizardPage(parent)
{
	_prevPage = prev;
	_nextPage = next;
	_typeSelectPage = typeSelectPage;
	_animGroup = animGroup;

	wxStaticText* pageLabel = new wxStaticText(this, wxID_ANY, _("Finally, name the animation."));
	wxStaticText* label = new wxStaticText(this, wxID_ANY, _("Animation name:"));
	wxTextCtrl* animNameCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200,20), 0, wxGenericValidator(&_animName));

	SetSizer(new wxBoxSizer(wxVERTICAL));
	GetSizer()->Add(pageLabel, 0, wxALL, 5);
	GetSizer()->AddStretchSpacer(1);
	wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
	row->Add(label, 0, wxALL|wxALIGN_CENTRE, 5);
	row->Add(animNameCtrl, 0, wxALL|wxALIGN_CENTRE|wxEXPAND, 5);
	GetSizer()->Add(row, 0, wxALIGN_CENTRE_VERTICAL, 5);
	GetSizer()->AddStretchSpacer(1);
}

void FxAnimWizardPageSelectName::SetPrev(wxWizardPage* prev)
{
	_prevPage = prev;
}

void FxAnimWizardPageSelectName::SetNext(wxWizardPage* next)
{
	_nextPage = next;
}

wxWizardPage* FxAnimWizardPageSelectName::GetPrev() const
{
	if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_AnalysisBased )
	{
		return _prevPage;
	}
	else if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_NonAnalysisBased )
	{
		if( _typeSelectPage->GetImportAudio() )
		{
#ifdef __UNREAL__
			return _typeSelectPage->GetNext()->GetNext();
#else
			return _typeSelectPage->GetNext();
#endif
		}
		else
		{
			return _typeSelectPage;
		}
	}
	else
	{
		return NULL;
	}
}

wxWizardPage* FxAnimWizardPageSelectName::GetNext() const
{
	return _nextPage;
}

FxString FxAnimWizardPageSelectName::GetAnimName() const
{
	return FxString(_animName.GetData());
}

void FxAnimWizardPageSelectName::OnPageChanging(wxWizardEvent& event)
{
	TransferDataFromWindow();

	if( event.GetDirection() )
	{
		// Check the destination group to see if the name is already in use.
		FxAnimGroup* _pAnimGroup = NULL;
		if( FxSessionProxy::GetAnimGroup(_animGroup, &_pAnimGroup) && _pAnimGroup )
		{
			FxSize index = _pAnimGroup->FindAnim(FxString(_animName.mb_str(wxConvLibc)).GetData());
			if( index != FxInvalidIndex )
			{
				wxString msg = wxString::Format(
					_("An animation named \"%s\" already exists in the group \"%s\". Overwriting an animation is not an undoable operation. Continue, and overwrite existing animation?"),
					_animName,
					wxString::FromAscii(_animGroup.GetData()));

				if( FxMessageBox(msg, _("Confirm Animation Overwrite"), wxYES_NO) == wxNO )
				{
					event.Veto();
				}
			}
		}
	}
}

void FxAnimWizardPageSelectName::OnPageChanged(wxWizardEvent& event)
{
	if( event.GetDirection() )
	{
		// Guess at the desired animation name.
		if( _typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_AnalysisBased ||
			_typeSelectPage->GetAnimType() == FxAnimWizardPageSelectType::AT_NonAnalysisBased && _typeSelectPage->GetImportAudio() )
		{
#ifdef __UNREAL__
			FxAnimWizardPageSelectSoundNodeWave* soundNodeWavePage = static_cast<FxAnimWizardPageSelectSoundNodeWave*>(_typeSelectPage->GetNext()->GetNext());
            FxString soundNodeWaveName = soundNodeWavePage->GetSelectedSoundNodeWave();
			_animName = wxString(soundNodeWaveName.GetData(), wxConvLibc);
#else
			// If the animation will have corresponding audio, use the base filename as the anim name.
			FxAnimWizardPageSelectAudio* audioPage = static_cast<FxAnimWizardPageSelectAudio*>(_typeSelectPage->GetNext());
			FxString audioFilename = audioPage->GetAudioFilename();
			_animName = wxFileNameFromPath(wxString(audioFilename.GetData(), wxConvLibc)).BeforeLast('.');
#endif
			TransferDataToWindow();
		}
	}
}

} // namespace Face
} // namespace OC3Ent
