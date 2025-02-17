//------------------------------------------------------------------------------
// A widget that uses the FaceFx Analysis Library to produce animation
// results.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxAnalysisWidget.h"
#include "FxVM.h"
#include "FxMath.h"
#include "FxAnimUserData.h"
#include "FxUndoRedoManager.h"
#include "FxSessionProxy.h"
#include "FxConsole.h"
#include "FxSecurityWrapper.h"
#include "FxStudioApp.h"
#include "FxCG.h"
#include "FxCGConfigManager.h"

namespace OC3Ent
{

namespace Face
{

FxReal FxAnalysisWidget::_audioMinimum = 0.5f;
FxReal FxAnalysisWidget::_audioMaximum = 90.0f;

// Function to log information from the analysis library.
void FxAnalysisLogOutput(const FxChar* msg, FxAnalysisLogChannel channel)
{
	FxString message("FxAnalysis: ");
	message << msg;
	switch(channel)
	{
	case FxAnalysisChannel_DevWarn:
		FxWarn(message.GetData());
		break;
	case FxAnalysisChannel_DevError:
		FxError(message.GetData());
		break;
	case FxAnalysisChannel_Warning:
		FxWarn(message.GetData());
		break;
	case FxAnalysisChannel_Error:
		FxError(message.GetData());
		break;
	case FxAnalysisChannel_DevTrace:
		// This spits out a LOT of information.
		break;
	case FxAnalysisChannel_Trace:
		FxMsg(message.GetData());
		break;
	};
}

FxAnalysisWidget::FxAnalysisWidget( FxWidgetMediator* mediator )
	: FxWidget(mediator)
	, _pAnimController(NULL)
	, _pActor(NULL)
	, _pAnim(NULL)
	, _pPhonemeList(NULL)
	, _pPhonemeMap(NULL)
{
}

FxAnalysisWidget::~FxAnalysisWidget()
{
}

void FxAnalysisWidget::SetAnimControllerPointer( FxAnimController* pAnimController )
{
	_pAnimController = pAnimController;
}

FxBool FxAnalysisWidget::IsExpired( void ) const
{
	return GetLicenseeInfo().isExpired;
}

FxAnalysisLicenseeInfo FxAnalysisWidget::GetLicenseeInfo( void ) const
{
	FxAnalysisLicenseeInfo licenseeInfo;
	FxAnalysisGetLicenseeInfo(FxWString(FxStudioApp::GetDataPath().mb_str(wxConvLibc)), licenseeInfo);
	return licenseeInfo;
}

void FxAnalysisWidget::OnAppStartup( FxWidget* FxUnused(sender) )
{
#ifndef __APPLE__
	FxMsg("Starting up FxAnalysis...");
	FxAnalysisSetLogOutputFunction(FxAnalysisLogOutput);

	// Display fonix version
	FxString versionString;
	FxAnalysisGetVersionString(versionString);
	FxMsg("%s", versionString.GetData());
		
	// Display the licensee info.
	FxAnalysisLicenseeInfo licenseeInfo = GetLicenseeInfo();

	wxString licensedTo = wxString::Format(wxT("Licensed to: %s"), wxString::FromAscii(licenseeInfo.licenseeName));
	wxString project = wxString::Format(wxT("Project: %s"),  wxString::FromAscii(licenseeInfo.licenseeProject));
	wxString expiration = wxString::Format(wxT("Expires: %s"),  wxString::FromAscii(licenseeInfo.expirationDate));
	FxMsg(licensedTo.mb_str(wxConvLibc));
	FxMsg(project.mb_str(wxConvLibc));
	FxMsg(expiration.mb_str(wxConvLibc));

	// Display the supported languages.
	FxArray<FxAnalysisLanguageInfo> supportedLanguages;
	FxAnalysisGetSupportedLanguages(FxWString(FxStudioApp::GetDataPath().mb_str(wxConvLibc)), supportedLanguages);

	FxSize numSupportedLanguages = supportedLanguages.Length();
	FxMsg("%d supported language%s:", numSupportedLanguages, numSupportedLanguages == 1 ? "" : "s");
	for( FxSize i = 0 ; i < numSupportedLanguages; ++i )
	{
		// Print the information to the console.
		wxString languageName(supportedLanguages[i].languageName.GetWCstr());
		wxString timeoutDate(supportedLanguages[i].timeoutDate.GetWCstr());
		if( supportedLanguages[i].isDefault )
		{
			FxMsg("%s (Default) [Timeout Date: %s]", languageName.mb_str(wxConvLibc), timeoutDate.mb_str(wxConvLibc));
		}
		else
		{
			FxMsg("%s [Timeout Date: %s]", languageName.mb_str(wxConvLibc), timeoutDate.mb_str(wxConvLibc));
		}
	}

	// Convert to the Studio-recognized format.
	_supportedLanguages.Clear();
	_supportedLanguages.Reserve(numSupportedLanguages);
	for( FxSize i = 0; i < numSupportedLanguages; ++i )
	{
		FxLanguageInfo language;
		language.languageCode = supportedLanguages[i].languageCode;
		language.languageName = supportedLanguages[i].languageName;
		language.nativeLanguageName = 
			supportedLanguages[i].nativeLanguageName;
		language.isDefault    = supportedLanguages[i].isDefault;
		_supportedLanguages.PushBack(language);
	}

	// Display the expired languages
	FxArray<FxAnalysisLanguageInfo> expiredLanguages;
	FxAnalysisGetExpiredLanguages(FxWString(FxStudioApp::GetDataPath().mb_str(wxConvLibc)), expiredLanguages);

	FxSize numExpiredLanguages = expiredLanguages.Length();
	FxMsg("%d expired language%s:", numExpiredLanguages, numExpiredLanguages == 1 ? "" : "s");
	for( FxSize i = 0 ; i < numExpiredLanguages; ++i )
	{
		// Print the information to the console.
		wxString languageName(expiredLanguages[i].languageName.GetWCstr());
		wxString timeoutDate(expiredLanguages[i].timeoutDate.GetWCstr());
		if( expiredLanguages[i].isDefault )
		{
			FxMsg("%s (Default) [Timeout Date: %s]", languageName.mb_str(wxConvLibc), timeoutDate.mb_str(wxConvLibc));
		}
		else
		{
			FxMsg("%s [Timeout Date: %s]", languageName.mb_str(wxConvLibc), timeoutDate.mb_str(wxConvLibc));
		}
	}

	// Set min and max console vars
	FxReal audioMin = 0.f, audioMax = 0.f;
	FxAnalysisGetParameter("audioMin", audioMin);
	FxAnalysisGetParameter("audioMax", audioMax);

	_audioMinimum = audioMin;
	_audioMaximum = audioMax;

	FxConsoleVariable* a_audiomin = FxVM::FindConsoleVariable("a_audiomin");
	FxConsoleVariable* a_audiomax = FxVM::FindConsoleVariable("a_audiomax");
	if( a_audiomin && a_audiomax )
	{
		a_audiomin->SetFloat(_audioMinimum);
		a_audiomax->SetFloat(_audioMaximum);
	}

	FxMsg( "Starting up FxCG..." );
	FxCGStartup();
	FxMsg( "Using FxCG version %f", static_cast<FxReal>(FxCGGetVersion()/1000.0f) );

#endif
}

void FxAnalysisWidget::OnAppShutdown( FxWidget* FxUnused(sender) )
{
	FxMsg( "Shutting down FxCG..." );
	FxCGShutdown();
	FxMsg( "FxCG shutdown successfully." );
}

void FxAnalysisWidget::OnActorChanged( FxWidget* FxUnused(sender), FxActor* actor )
{
	_pActor = actor;
}

void FxAnalysisWidget::
OnQueryLanguageInfo( FxWidget* FxUnused(sender), FxArray<FxLanguageInfo>& languages )
{
	FxAssert(_supportedLanguages.Length() > 0);
	languages = _supportedLanguages;
}

void FxAnalysisWidget::
OnCreateAnim( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim )
{
	FxBool canContinue = FxTrue;
	FxIsLicensed(canContinue);

	_pAnim = anim;

	if( _pActor && _pAnim && canContinue )
	{
		_setSpeechDetectionState();

		// The sender of this message should have verified that the animation
		// group exists, the animation will not be a duplicate, and that the 
		// animation user data was created with a valid digital audio pointer
		// (including resampling) and with valid analysis text (if any).
		FxAnimUserData* pUserData = reinterpret_cast<FxAnimUserData*>(_pAnim->GetUserData());
		if( pUserData && pUserData->ShouldContainAnalysisResults() )
		{
			FxAnalysisAudio analysisAudio;
			FxWString analysisText;
			FxAnalysisConfig analysisConfig;
			FxAnalysisResults analysisResults;

			_SetupAnalysis(pUserData, analysisAudio, analysisText, analysisConfig);

			FxAnalysisError err = FxAnalyze(analysisAudio,
											analysisText,
											analysisConfig,
											analysisResults);
			if( err == FxAnalysisError_None )
			{
				// Add the curves to the animation.
				FxAnim& resultAnim = analysisResults.animation;
				for( FxSize i = 0; i < resultAnim.GetNumAnimCurves(); ++i )
				{
					_pAnim->AddAnimCurve(resultAnim.GetAnimCurve(i));
				}

				// Construct the phoneme/word list.
				FxPhonWordList* pPhonemeWordList = new FxPhonWordList();
				// Set the phoneme min time to something we query from the 
				// analysis library.
				FxReal minPhonDuration = 0.01f;
				FxAnalysisGetParameter("minPhonemeDuration", minPhonDuration);
				pPhonemeWordList->SetMinPhonemeLength(minPhonDuration);

				FxArray<FxPhoneTimeInfo>& phoneTimes = analysisResults.phonemeTimes;
				FxArray<FxWordTimeInfo>&  wordTimes  = analysisResults.wordTimes;
				for( FxSize i = 0; i < phoneTimes.Length(); ++i )
				{
					pPhonemeWordList->AppendPhoneme(
						phoneTimes[i].phoneEnum,
						phoneTimes[i].phoneStart,
						phoneTimes[i].phoneEnd);
				}
				for( FxSize i = 0; i < wordTimes.Length(); ++i )
				{
					pPhonemeWordList->GroupToWord(
						wxString(wordTimes[i].wordText.GetData()),
						wordTimes[i].wordStart,
						wordTimes[i].wordEnd);
				}

				pPhonemeWordList->ClearDirtyFlag();
				pUserData->SetPhonemeWordList(pPhonemeWordList);
				delete pPhonemeWordList; pPhonemeWordList = NULL;

				// Mark the correct curves as owned by analysis.
				for( FxSize i = 0; i < _pPhonemeMap->GetNumTargets(); ++i )
				{
					pUserData->SetCurveOwnedByAnalysis(_pPhonemeMap->GetTargetName(i), FxTrue);
				}

				// Only mark the spech gesture curves as owned by analysis if 
				// they were in fact generated by the analysis.
				if( pUserData->ShouldContainSpeechGestures() )
				{
					for( FxSize i = 0; i < FxCGGetNumSpeechGestureTracks(); ++i )
					{
						pUserData->SetCurveOwnedByAnalysis(FxCGGetSpeechGestureTrackName(i), FxTrue);
					}
				}

				_pAnimController->ProofUserData();
				FxSessionProxy::ActorDataChanged(ADCF_CurvesChanged);
				_pAnim->FindBounds();
			}
			else
			{
				switch(err)
				{
				case FxAnalysisError_InvalidBitsPerSample:
					FxMessageBox(_("Invalid Audio: audio must be 16 bits per sample."), _("Analysis Error"));
					break;
				case FxAnalysisError_InvalidSampleRate:
					FxMessageBox(_("Invalid Audio: audio sample rate must be 16000 or 8000."), _("Analysis Error"));
					break;
				case FxAnalysisError_InvalidChannelCount:
					FxMessageBox(_("Invalid Audio: audio must be mono."), _("Analysis Error"));
					break;
				case FxAnalysisError_DataNotFound:
					FxMessageBox(_("The analysis data could not be found. Please reinstall FaceFX Studio"), _("Analysis Error"));
				default:
					break;
				};
				FxError( "<b>FxAnalysisWidget: failed to create animation!</b>" );
			}

			// Free the sample buffer.
			delete[] analysisAudio.pAudio; analysisAudio.pAudio = NULL;
		}		
	}
	else
	{
		FxWarn( "<b>FxAnalysisWidget::OnCreateAnim - animation or actor pointer is NULL!</b>" );
	}

}

void FxAnalysisWidget::
OnAudioMinMaxChanged( FxWidget* FxUnused(sender), FxReal audioMin, FxReal audioMax )
{
	FxAnalysisSetParameter("audioMin", audioMin);
	FxAnalysisSetParameter("audioMax", audioMax);

	FxReal newAudioMin = 0.f, newAudioMax = 0.f;
	FxAnalysisGetParameter("audioMin", newAudioMin);
	FxAnalysisGetParameter("audioMax", newAudioMax);

	_audioMinimum = newAudioMin;
	_audioMaximum = newAudioMax;

	FxConsoleVariable* a_audiomin = FxVM::FindConsoleVariable("a_audiomin");
	FxConsoleVariable* a_audiomax = FxVM::FindConsoleVariable("a_audiomax");
	if( a_audiomin && a_audiomax )
	{
		a_audiomin->SetFloat(_audioMinimum);
		a_audiomax->SetFloat(_audioMaximum);
	}
}

void FxAnalysisWidget::
OnReanalyzeRange( FxWidget* FxUnused(sender), FxAnim* anim, FxReal rangeStart, 
				  FxReal rangeEnd, const FxWString& analysisText )
{
	if( _pActor && anim && anim == _pAnim )
	{
		_setSpeechDetectionState();

		FxAnimUserData* pUserData = reinterpret_cast<FxAnimUserData*>(anim->GetUserData());
		if( pUserData && pUserData->ShouldContainAnalysisResults() )
		{

			FxAnalysisAudio analysisAudio;
			FxWString textToAnalyze;
			FxAnalysisConfig analysisConfig;
			FxAnalysisResults analysisResults;

			_SetupAnalysis(pUserData, analysisAudio, textToAnalyze, analysisConfig);

			// Override the analysis text.
			textToAnalyze = analysisText;

			// Override the audio information based on the range to reanalyze.
			// Cache the original pointer to delete later.
			const FxByte* originalAudioPointer = analysisAudio.pAudio; 
			FxSize startSample = static_cast<FxSize>(rangeStart * 
								 analysisAudio.sampleRate);
			FxSize endSample   = static_cast<FxSize>(rangeEnd * 
								 analysisAudio.sampleRate);

			FxBool canContinue = FxTrue;
			if( startSample > analysisAudio.numSamples-1 ||
				endSample > analysisAudio.numSamples-1 )
			{
				canContinue = FxFalse;
			}

			analysisAudio.pAudio += startSample * 
				(analysisAudio.bitsPerSample / FX_CHAR_BIT);
			analysisAudio.numSamples = endSample - startSample + 1;

			if( canContinue && analysisAudio.numSamples > 1 )
			{
				FxAnalysisError err = FxAnalyzeAudio(analysisAudio,
													analysisText,
													analysisConfig,
													analysisResults);
				if( err == FxAnalysisError_None )
				{
					// Construct the phoneme/word list.
					FxPhonWordList tempPhonWordList;
					// Set the phoneme min time to something we query from the 
					// analysis library
					FxReal minPhonDuration = 0.01f;
					FxAnalysisGetParameter("minPhonemeDuration", minPhonDuration);
					tempPhonWordList.SetMinPhonemeLength(minPhonDuration);

					FxArray<FxPhoneTimeInfo>& phoneTimes = analysisResults.phonemeTimes;
					FxArray<FxWordTimeInfo>&  wordTimes  = analysisResults.wordTimes;
					for( FxSize i = 0; i < phoneTimes.Length(); ++i )
					{
						tempPhonWordList.AppendPhoneme(
							phoneTimes[i].phoneEnum,
							phoneTimes[i].phoneStart,
							phoneTimes[i].phoneEnd);
					}
					for( FxSize i = 0; i < wordTimes.Length(); ++i )
					{
						tempPhonWordList.GroupToWord(
							wxString(wordTimes[i].wordText.GetData()),
							wordTimes[i].wordStart,
							wordTimes[i].wordEnd);
					}
					
					_PostProcessPhonemeList(&tempPhonWordList, 
											rangeEnd - rangeStart);

					// Splice the new phoneme list back into the old phoneme list.
					FxPhonWordList* pPhonemeWordList = 
						pUserData->GetPhonemeWordListPointer();
					FxSize startIndex = pPhonemeWordList->FindPhoneme(rangeStart);
					FxSize endIndex   = pPhonemeWordList->FindPhoneme(rangeEnd);
					pPhonemeWordList->Splice(tempPhonWordList, startIndex, endIndex);

					// This is not an undoable operation, so the undo / redo buffers 
					// should be flushed.
					FxUndoRedoManager::Flush();
					// The session should also be flagged as forced dirty.
					FxSessionProxy::SetIsForcedDirty(FxTrue);

					// The analysis text could have changed, so we should update 
					// it with the words from the phoneme word list here.
					if( pPhonemeWordList )
					{
						wxString newAnalysisText;
						for( FxSize i = 0; i < pPhonemeWordList->GetNumWords(); ++i )
						{
							newAnalysisText.Append(pPhonemeWordList->GetWordText(i));
							newAnalysisText.Append(wxT(" "));
						}
						// Remove that last space if it's been appended.
						if( newAnalysisText != wxEmptyString )
						{
							newAnalysisText.RemoveLast();
						}
						pUserData->SetAnalysisText(FxWString(newAnalysisText.GetData()));
						pUserData->SetIsSafeToUnload(FxFalse);
					}

					// Update the widgets, and the animation, with the new 
					// phoneme & word list.
					if( _mediator )
					{
						_mediator->OnAnimPhonemeListChanged(NULL, _pAnim, 
															pPhonemeWordList);
					}

					// Find the bounds of the animation.
					_pAnim->FindBounds();
				}
				else
				{
					FxError( "<b>FxAnalysisWidget: failed to reanalyze range!</b>" );
				}
			}
			else
			{
				FxError("FxAnalysisWidget: failed to reanalyze range. Selected range is outside the bounds of the audio.");
			}

			// Free the sample buffer.
			delete[] originalAudioPointer; originalAudioPointer = NULL;
		}
		else
		{
			FxError( "<b>FxAnalysisWidget: failed to reanalyze range!</b>" );
		}
	}
	else
	{
		FxWarn( "<b>FxAnalysisWidget::OnReanalyzeRange - animation or actor pointer is NULL or animation pointer mismatch!</b>" );
	}
}

void FxAnalysisWidget::
OnAnimChanged( FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim )
{
	_pAnim = anim;
}

void FxAnalysisWidget::
OnAnimPhonemeListChanged( FxWidget* FxUnused(sender), FxAnim* anim, FxPhonWordList* phonemeList )
{
	_pPhonemeList = phonemeList;
	if( _pAnim && _pAnim == anim && _pPhonemeList )
	{
		FxAnimUserData* pUserData = reinterpret_cast<FxAnimUserData*>(_pAnim->GetUserData());
		if( pUserData && pUserData->ShouldContainAnalysisResults() )
		{
			FxAnalysisAudio analysisAudio;
			FxWString analysisText;
			FxAnalysisConfig analysisConfig;
			FxAnalysisResults analysisResults;

			_SetupAnalysis(pUserData, analysisAudio, analysisText, analysisConfig);

			// Fill out the phoneme times in the analysis results.
			FxPhonWordList* pPhonWordList = pUserData->GetPhonemeWordListPointer();
			for( FxSize i = 0; i < pPhonWordList->GetNumPhonemes(); ++i )
			{
				analysisResults.phonemeTimes.PushBack(
					FxPhoneTimeInfo(
						pPhonWordList->GetPhonemeEnum(i),
						pPhonWordList->GetPhonemeStartTime(i),
						pPhonWordList->GetPhonemeEndTime(i)));
			}
			// Fill out the word times in the analysis results.
			for( FxSize i = 0; i < pPhonWordList->GetNumWords(); ++i )
			{
				analysisResults.wordTimes.PushBack(
					FxWordTimeInfo(
						pPhonWordList->GetWordText(i).GetData(),
						pPhonWordList->GetWordStartTime(i),
						pPhonWordList->GetWordEndTime(i)));
			}

			// Analyze the phonemes to re-coarticulate and get updated
			// speech gestures.
			FxAnalysisError err = 
				FxAnalyzePhonemes(analysisAudio, analysisText, 
								  analysisConfig, analysisResults);
			if( err == FxAnalysisError_None )
			{
				// Update the animation.
				FxAnim& resultAnim = analysisResults.animation;
				for( FxSize i = 0; i < resultAnim.GetNumAnimCurves(); ++i )
				{
					const FxAnimCurve& resultCurve = resultAnim.GetAnimCurve(i);
					// Find the curve in the animation to update.
					FxSize index = _pAnim->FindAnimCurve(resultCurve.GetName());
					if( index != FxInvalidIndex &&
						_pAnimController->IsCurveOwnedByAnalysis(index) )
					{
						// Clean up the user data on the keys.
						_pAnimController->DestroyCurveUserData(index);
						FxAnimCurve& curveInAnim = _pAnim->GetAnimCurveM(index);
						curveInAnim.RemoveAllKeys();

						// Insert keys in reverse order for a speed up on key insertion.
						for( FxInt32 currentKey = resultCurve.GetNumKeys() - 1; currentKey >= 0; --currentKey )
						{
							curveInAnim.InsertKey(resultCurve.GetKey(currentKey));
						}
					}
				}
				_pAnimController->ProofUserData();
			}
			else
			{
				FxError("<b>FxAnalysisWidget: failed to analyze new phoneme list</b>");
			}

			// Free the sample buffer.
			delete[] analysisAudio.pAudio; analysisAudio.pAudio = NULL;
		}
	}
}

void FxAnalysisWidget::OnPhonemeMappingChanged( FxWidget* FxUnused(sender), FxPhonemeMap* phonemeMap )
{
	_pPhonemeMap = phonemeMap;
	if( _pPhonemeMap && _pAnimController )
	{
		// Force the animation curves owned by the analysis to stay synchronized
		// with the new mapping.
		OnAnimPhonemeListChanged(NULL, _pAnim, _pPhonemeList);
	}
}

void FxAnalysisWidget::OnGenericCoarticulationConfigChanged( FxWidget* FxUnused(sender), FxCoarticulationConfig* FxUnused(config) )
{
	// Now unneeded.
}

void FxAnalysisWidget::OnGestureConfigChanged( FxWidget* FxUnused(sender), FxGestureConfig* FxUnused(config) )
{
	// Now unneeded.
}

void FxAnalysisWidget::OnAudioMinimumUpdated( FxConsoleVariable& cvar )
{
	_audioMinimum = static_cast<FxReal>(cvar);
	FxSessionProxy::ChangeAudioMinMax(_audioMinimum, _audioMaximum);
}

void FxAnalysisWidget::OnAudioMaximumUpdated( FxConsoleVariable& cvar )
{
	_audioMaximum = static_cast<FxReal>(cvar);
	FxSessionProxy::ChangeAudioMinMax(_audioMinimum, _audioMaximum);
}

void FxAnalysisWidget::_setSpeechDetectionState()
{
	FxConsoleVariable* cvar = FxVM::FindConsoleVariable("a_detectspeech");
	if( cvar )
	{
		FxString val = cvar->GetString();
		if( val == "yes" || val == "true" || val == "on" || val == "1" || 
		    val == "YES" || val == "TRUE" || val == "ON" )              
		{
			FxAnalysisSetParameter("speechDetection", 1.0f);
		}
		else if( val == "no" || val == "false" || val == "off" || val == "0" ||
			     val == "NO" || val == "FALSE" || val == "OFF" )
		{
			FxAnalysisSetParameter("speechDetection", 0.0f);
		}
		else
		{
			FxAnalysisSetParameter("speechDetection", -1.0f);
		}
	}
}

void FxAnalysisWidget::_SetupAnalysis( FxAnimUserData* pUserData,
									   FxAnalysisAudio& analysisAudio,
									   FxWString& analysisText,
									   FxAnalysisConfig& analysisConfig )
{
	analysisText = pUserData->GetAnalysisText();

	// Find the data path.
	wxFileName dataPath(FxStudioApp::GetAppDirectory());
	dataPath.SetFullName(_("FonixData.cdf"));
	FxWString dataDirectory(dataPath.GetFullPath().GetData());

	// Set up for analysis.
	FxAudio* pAudio = pUserData->GetAudioPointer();
	if( pAudio )
	{
		FxRawSampleBuffer rawSampleBuffer = pAudio->ExportRawSampleBuffer();
		// Make a deep copy of the sample buffer to pass off to the analysis chain.
		FxSize bufferSize = FxGetBufferSize(rawSampleBuffer);
		FxByte* pBuffer = new FxByte[bufferSize];
		FxMemcpy(pBuffer, rawSampleBuffer.GetSamples(), bufferSize);

		analysisAudio = FxAnalysisAudio(
			pBuffer,
			pAudio->GetNumSamples(),
			pAudio->GetSampleRate(),
			pAudio->GetBitsPerSample(),
			pAudio->GetNumChannels());
	}
	else
	{
		analysisAudio = FxAnalysisAudio();
	}

	// Set the configurations.
	FxCoarticulationConfig* pCoarticulationConfig = 
		FxCGConfigManager::GetGenericCoarticulationConfig(pUserData->GetCoarticulationConfig().GetData());
	FxGestureConfig* pGestureConfig = 
		FxCGConfigManager::GetGestureConfig(pUserData->GetGestureConfig().GetData());
	//@todo Fix this at some point.
	// The previous call to FxCGConfigManager::GetGestureConfig() could have caused all of the
	// configurations to be rescanned in the rare case that the gesture configuration could not
	// be found.  Since those are stored as arrays, the previous pointer is no longer valid, so
	// cache it again.
	pCoarticulationConfig = 
		FxCGConfigManager::GetGenericCoarticulationConfig(pUserData->GetCoarticulationConfig().GetData());

	analysisConfig.dataPath = dataDirectory;
	analysisConfig.pPhonemeMap = _pPhonemeMap;
	analysisConfig.language = pUserData->GetLanguage();
	analysisConfig.pCoarticulationConfig = pCoarticulationConfig;
	analysisConfig.pGestureConfig = pGestureConfig;
	analysisConfig.generateSpeechGestures = pUserData->ShouldContainSpeechGestures();
	analysisConfig.randomSeed = pUserData->GetRandomSeed();
}

void FxAnalysisWidget::_PostProcessPhonemeList( FxPhonWordList* pPhonList, FxReal audioEndTime )
{
	for( FxSize i = 0; i < pPhonList->GetNumPhonemes(); ++i )
	{
		// Remove any phoneme entirely before 0.
		if( pPhonList->GetPhonemeEndTime(i) < 0.f )
		{
			pPhonList->RemovePhoneme(i);
		}

		// Remove any phonemes entirely after the end time of the audio.
		if( pPhonList->GetPhonemeStartTime(i) > audioEndTime )
		{
			pPhonList->RemovePhoneme(i);
		}
	}

	if( pPhonList->GetNumPhonemes() )
	{
		// Truncate first phoneme to start at 0.
		FxReal newStartTime = pPhonList->ModifyPhonemeStartTime(0, 0.f);
		//@todo hack - this will only work if the next phoneme can give away
		// min phoneme length worth of it's own duration.  otherwise, it's still
		// going to yield some busted phoneme lists once we start splicing the
		// results of reanalyze range into pre-existing phoneme list.
		if( newStartTime < 0.0f )
		{
			pPhonList->ModifyPhonemeEndTime(0, 0.f + pPhonList->GetMinPhonemeLength());
			pPhonList->ModifyPhonemeStartTime(0, 0.f);
		}

		// Truncate the last phoneme to end at the audio's end time.
		pPhonList->ModifyPhonemeEndTime(pPhonList->GetNumPhonemes() - 1, audioEndTime);
	}
}

} // namespace Face

} // namespace OC3Ent
