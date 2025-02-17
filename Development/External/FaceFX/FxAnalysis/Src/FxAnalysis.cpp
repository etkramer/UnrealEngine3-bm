//------------------------------------------------------------------------------
// A component for generating FaceFX animations outside of FaceFX Studio.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxAnalysis.h"
#include "FxUnicodeTextTagParser.h"
#include "FxAnalysisFonix.h"
#include "FxCG.h"
#include "FxSpeechGestures.h"
#include "FxCoarticulationGeneric.h"
#include "FxSimplePhonemeList.h"
#include "FxCoarticulationConfig.h"
#include "FxGestureConfig.h"

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Logging supporting code
//------------------------------------------------------------------------------

// A pointer to the log output function.
static FxAnalysisLogOutputFunc g_logFunction = NULL;

#define FxAnalysisLog(chan, msg) \
	if( g_logFunction ) \
	{ \
		g_logFunction((FxString() << msg).GetData(), chan);\
	} \

#define FxAnalysisLogError(msg) FxAnalysisLog(FxAnalysisChannel_Error, msg)
#define FxAnalysisLogWarning(msg) FxAnalysisLog(FxAnalysisChannel_Warning, msg)
#define FxAnalysisLogTrace(msg) FxAnalysisLog(FxAnalysisChannel_Trace, msg)
#define FxAnalysisLogDevError(msg) FxAnalysisLog(FxAnalysisChannel_DevError, msg)
#define FxAnalysisLogDevWarn(msg) FxAnalysisLog(FxAnalysisChannel_DevWarn, msg)
#define FxAnalysisLogDevTrace(msg) FxAnalysisLog(FxAnalysisChannel_DevTrace, msg)

// Checks for FaceFX Fonix Analysis Library errors and dumps the specified 
// message on success.
#define CHECK_FXAL_ERROR_NOTIFY_SUCCESS(errorCode, successMsg) \
	if( FX_ANALYSIS_FONIX_NOERROR != errorCode ) \
	{ \
		FxInt32 errorStringLength = 1023; \
		FxWChar  errorString[1024] = {0}; \
		FxAnalysisFonixGetErrorString(errorCode, errorStringLength, \
                                      errorString); \
		FxString converter(errorString); \
		FxAnalysisLogError(converter.GetData()); \
	} \
	else \
	{ \
		FxAnalysisLogTrace( successMsg ); \
	}

// Checks for FaceFX Fonix Analysis Library errors and does nothing on success.
#define CHECK_FXAL_ERROR(errorCode) \
	if( FX_ANALYSIS_FONIX_NOERROR != errorCode ) \
	{ \
		FxInt32 errorStringLength = 1023; \
		FxWChar  errorString[1024] = {0}; \
		FxAnalysisFonixGetErrorString(errorCode, errorStringLength, \
									  errorString); \
		FxString converter(errorString); \
		FxAnalysisLogError(converter.GetData()); \
	}

//------------------------------------------------------------------------------
// Supporting structures
//------------------------------------------------------------------------------

FxAnalysisAudio::FxAnalysisAudio(const FxByte* ipAudio,
								 FxSize        iNumSamples,
								 FxSize        iSampleRate,
								 FxSize        iBitsPerSample,
								 FxSize        iChannelCount)
	: pAudio(ipAudio)
	, numSamples(iNumSamples)
	, sampleRate(iSampleRate)
	, bitsPerSample(iBitsPerSample)
	, channelCount(iChannelCount)
{
}

FxAnalysisConfig::FxAnalysisConfig()
	: pPhonemeMap(NULL)
	, language("USEnglish")
	, pCoarticulationConfig(NULL)
	, pGestureConfig(NULL)
	, generateSpeechGestures(FxTrue)
	, randomSeed(static_cast<FxInt32>(time(0)))
{
}

FxPhoneTimeInfo::FxPhoneTimeInfo(FxPhoneme iPhoneEnum, 
								 FxReal    iPhoneStart, 
								 FxReal    iPhoneEnd)
	: phoneEnum(iPhoneEnum)
	, phoneStart(iPhoneStart)
	, phoneEnd(iPhoneEnd)
{
}

FxWordTimeInfo::FxWordTimeInfo(const FxWString& iWordText,
							   FxReal           iWordStart,
							   FxReal           iWordEnd)
	: wordText(iWordText)
	, wordStart(iWordStart)
	, wordEnd(iWordEnd)
{
}

//------------------------------------------------------------------------------
// Helper function declarations
//------------------------------------------------------------------------------

// Processes logging information from the underlying library.
void			FX_CALL _LogOutput(
							const wchar_t*			  msg,
							FxAnalysisFonixLogChannel chan);

// Checks if a given language is supported. Fills out language code to the 
// correct value if the language is supported.
FxBool          FX_CALL _IsLanguageSupported(
							const FxAnalysisConfig& analysisConfig,
							FxInt32&				languageCode );

// Starts up the analysis library.
FxAnalysisError FX_CALL _StartupAnalysis(
							 const FxAnalysisConfig& analysisConfig );

// Send the audio to the analysis library.
FxAnalysisError FX_CALL _ProcessAudio(
							 const FxAnalysisAudio& audioInformation,
							 const FxWString&		text,
							 const FxInt32			languageCode,
							 FxAnalysisResults&		analysisResults );

// Shuts down the analysis library.
FxAnalysisError FX_CALL _ShutdownAnalysis();

// Proofs the audio file format and returns an error code if something isn't
// going to play nicely with the analysis engine.
FxAnalysisError FX_CALL _ProofAudio(
							const FxAnalysisAudio&	audioInformation);

// Runs coarticulation on the analysis results.
FxAnalysisError FX_CALL _RunCoarticulation(
							FxAnalysisConfig&		analysisConfig,
							FxAnalysisResults&		analysisResults);

// Runs speech gestures on the analysis results.
FxAnalysisError FX_CALL _RunSpeechGestures(
							const FxAnalysisAudio&	audioInformation,
							FxAnalysisConfig&		analysisConfig,
							FxAnalysisResults&		analysisResults );

// Returns a sample buffer suitable for speech gesture processing.
// The returned buffer should be freed with delete[].
FxDReal*        FX_CALL _CreateSampleBuffer(
							const FxAnalysisAudio&	audioInformation );

// Processes the text tags.
FxAnalysisError FX_CALL _ProcessTextTags(
							const FxWString&		text,
							FxAnalysisResults&		analysisResults );

// Analyze an audio file with text.
FxAnalysisError FX_CALL __AnalyzeWithText(
							const FxAnalysisAudio&	audioInformation,
							const FxWString&		text,
							const FxInt32			languageCode,
							FxAnalysisResults&		analysisResults );

// Chunk an audio file without text.
FxAnalysisError FX_CALL __ChunkAnalyze(
							const FxAnalysisAudio&	audioInformation,
							const FxInt32			languageCode,
							FxAnalysisResults&		analysisResults );

// Analyze a single chunk of the audio, bounded by the start and end times.
FxAnalysisError FX_CALL ___AnalyzeSingleChunk(
							const FxAnalysisAudio&	audioInformation,
							const FxInt32			languageCode,
							const FxDReal			chunkStart,
							const FxDReal			chunkEnd,
							FxAnalysisResults&		analysisResults );

//------------------------------------------------------------------------------
// Global variables.
//------------------------------------------------------------------------------
static FxBool g_speechDetectionEnabled = FxTrue;

//------------------------------------------------------------------------------
// Error code information
//------------------------------------------------------------------------------
FxString FxAnalysisGetErrorInformation(FxAnalysisError errCode)
{
	switch(errCode)
	{
	case FxAnalysisError_None:
		return "No error.";
	case FxAnalysisError_LicenseExpired:
		return "Your license to FxAnalysis has expired. Please contact support@oc3ent.com to discuss purchasing FaceFx";
	case FxAnalysisError_NullParameter:
		return "A required parameter was NULL.";
	case FxAnalysisError_InvalidBitsPerSample:
		return "The specified bits per sample is unsupported. Bits per sample must be 16";
	case FxAnalysisError_InvalidSampleRate:
		return "The specified sample rate is unsupported. Sample rate must be 8000 or 16000";
	case FxAnalysisError_InvalidChannelCount:
		return "The channel count is invalid. Channel count must be 1 (mono).";
	case FxAnalysisError_InvalidLanguage:
		return "The specified language is not supported.";
	case FxAnalysisError_DataNotFound:
		return "The analysis data file was not found.";
	case FxAnalysisError_InvalidParameterName:
		return "A parameter with the given name does not exist.";
	case FxAnalysisError_InternalError:
		return "An internal error occurred. Please contact support@oc3ent.com with a description of how you encountered this error.";
	default:
		return "Invalid FxAnalysisError enumeration value.";
	};
}

//------------------------------------------------------------------------------
// One-step analysis
//------------------------------------------------------------------------------
FxAnalysisError FX_CALL FxAnalyze( 
							const FxAnalysisAudio& audioInformation,
							const FxWString&       text,
							FxAnalysisConfig&      analysisConfig,
							FxAnalysisResults&     analysisResults )
{
	// Process the audio.
	FxAnalysisError retval = FxAnalyzeAudio(audioInformation, text,
											analysisConfig, analysisResults);

	// If audio processing wasn't successful, abort and notify the user.
	if( retval != FxAnalysisError_None )
	{
		if( retval == FxAnalysisError_InvalidLanguage )
		{
			FxAnalysisLogError("Could not analyze audio: the desired language is invalid or expired.");
		}
		else
		{
			FxAnalysisLogError("Could not analyze audio.");
		}
		return retval;
	}

	// Otherwise, attempt to analyze the phonemes and create the animation.
	FxAnalysisLogTrace("Processing phonetic analysis.");
	retval =  FxAnalyzePhonemes(audioInformation, text,
							    analysisConfig, analysisResults);

	if( retval == FxAnalysisError_None )
	{
		FxAnalysisLogTrace("Animation created successfully!");
	}
	else
	{
		FxAnalysisLogError("Animation was not created successfully!");
	}

	return retval;
}

//------------------------------------------------------------------------------
// Two-step analysis
//------------------------------------------------------------------------------
FxAnalysisError FX_CALL FxAnalyzeAudio( 
							const FxAnalysisAudio& audioInformation,
							const FxWString&       text,
							FxAnalysisConfig&      analysisConfig,
							FxAnalysisResults&     analysisResults )
{
	FxAnalysisLogTrace("Processing audio analysis.");

	// Check that the audio is of the correct format.  If it's not, abort.
	FxAnalysisError retval = _ProofAudio(audioInformation);
	if( retval != FxAnalysisError_None )
	{
		return retval;
	}

	// Check that the language passed in is supported.  If it's not, abort.
	FxInt32 languageCode = -1;
	if( !_IsLanguageSupported(analysisConfig, languageCode) )
	{
		return FxAnalysisError_InvalidLanguage;
	}

	FxWString message("Starting analysis with data directory: ");
	message += analysisConfig.dataPath;
	FxAnalysisLogTrace(message.GetCstr());

	// Attempt to start up the analysis.  If it fails, abort.
	retval = _StartupAnalysis(analysisConfig);
	if( retval != FxAnalysisError_None )
	{
		return retval;
	}

	FxWString taglessText;
	if( text.Length() )
	{
		// Strip the text tags from the text in preparation for analysis.
		FxUnicodeTextTagParser parser(text);
		taglessText = parser.GetStrippedText();
	}

	// Clear the analysis results in preparation for receiving their data.
	analysisResults.phonemeTimes.Clear();
	analysisResults.wordTimes.Clear();

	// Process the audio.
	retval = _ProcessAudio(audioInformation, taglessText, languageCode, 
		                   analysisResults);

	// Shut down the analysis.
	retval = _ShutdownAnalysis();

	return retval;
}

FxAnalysisError FX_CALL FxAnalyzePhonemes(
							const FxAnalysisAudio& audioInformation,
							const FxWString&       text,
							FxAnalysisConfig&      analysisConfig,
							FxAnalysisResults&     analysisResults )
{
	// Check that the audio is of the correct format.  If it's not, abort.
	FxAnalysisError retval = _ProofAudio(audioInformation);
	if( !(retval == FxAnalysisError_None ||
		  retval == FxAnalysisError_NullParameter) )
	{
		return retval;
	}
	if( retval == FxAnalysisError_NullParameter )
	{
		FxAnalysisLogWarning("Speech gestures will not be processed without "
							 "an audio stream to which to align the gestures.");
	}

	// Check that we have a valid phoneme to track mapping.
	if( analysisConfig.pPhonemeMap == NULL )
	{
		FxAnalysisLogError("The pointer to the phoneme map must be valid.");
		return FxAnalysisError_NullParameter;
	}

	// If the user didn't fill in a coarticulation or gesture config pointer, 
	// do so for them.
	FxCoarticulationConfig defaultCoarticulationConfig;
	FxGestureConfig        defaultGestureConfig;
	if( analysisConfig.pCoarticulationConfig == NULL )
	{
		analysisConfig.pCoarticulationConfig = &defaultCoarticulationConfig;
	}
	if( analysisConfig.pGestureConfig == NULL )
	{
		analysisConfig.pGestureConfig = &defaultGestureConfig;
	}

	// Reset the animation so it will be ready receive results.
	// Cache the anim name in case the user expects that behaviour.
	FxName animName = analysisResults.animation.GetName();;
	analysisResults.animation = FxAnim();
	analysisResults.animation.SetName(animName);

	// Run coarticulation and add its curves to the resulting animation.
	retval = _RunCoarticulation(analysisConfig, analysisResults);
	if( retval != FxAnalysisError_None )
	{
		return retval;
	}

	if( audioInformation.pAudio && analysisConfig.generateSpeechGestures )
	{
		// Run speech gestures and add its curves to the resulting animation.
		retval = _RunSpeechGestures(audioInformation, 
									analysisConfig, 
									analysisResults );
		if( retval != FxAnalysisError_None )
		{
			return retval;
		}
	}

	// Process the text tags.
	retval = _ProcessTextTags(text, analysisResults);

	return retval;
}

//------------------------------------------------------------------------------
// Logging
//------------------------------------------------------------------------------

FxAnalysisError FX_CALL FxAnalysisSetLogOutputFunction(
							FxAnalysisLogOutputFunc pLogOutputFunc )
{
	g_logFunction = pLogOutputFunc;
	FxAnalysisLogTrace("Log callback registered.");

	if( g_logFunction )
	{
		FxAnalysisFonixSetLogOutputFunction(_LogOutput);
	}
	else
	{
		FxAnalysisFonixSetLogOutputFunction(NULL);
	}

	return FxAnalysisError_None;
}

//------------------------------------------------------------------------------
// Licensee and library information
//------------------------------------------------------------------------------

FxAnalysisError FX_CALL FxAnalysisGetVersionString(
							FxString& versionString)
{
	FxInt32 fonixVersion;
	FxAnalysisFonixGetVersion(fonixVersion);

	versionString = "Fonix analysis library version ";
	versionString << fonixVersion;

	return FxAnalysisError_None;
}

// Default constructor.
FxAnalysisLicenseeInfo::FxAnalysisLicenseeInfo()
	: isExpired(FxTrue) // Default to expired, just in case.
{
	// Clear the fields.
	FxMemset(licenseeName, 0, FxAnalysisLicenseeInfoMaxLen);
	FxMemset(licenseeProject, 0, FxAnalysisLicenseeInfoMaxLen);
	FxMemset(expirationDate, 0, FxAnalysisLicenseeInfoMaxLen);
}

FxAnalysisError FX_CALL FxAnalysisGetLicenseeInfo(
							const FxWString& dataPath,
							FxAnalysisLicenseeInfo& licenseeInfo )
{
	FxAnalysisConfig config;
	config.dataPath = dataPath;
	FxAnalysisError retval = _StartupAnalysis(config);
	if( retval == FxAnalysisError_None ||
		retval == FxAnalysisError_LicenseExpired )
	{
		// Clear the licensee info struct.
		licenseeInfo = FxAnalysisLicenseeInfo();

		FxAnalysisFonixLicenseeInfo fonixLicenseeInfo;
		FxAnalysisFonixGetLicenseeInfo(fonixLicenseeInfo);

		FxStrncpy(licenseeInfo.licenseeName,
				FxAnalysisLicenseeInfoMaxLen,
				fonixLicenseeInfo.licenseeName,
				FxAnalysisLicenseeInfoMaxLen);
		licenseeInfo.licenseeName[FxAnalysisLicenseeInfoMaxLen] = 0;
		FxStrncpy(licenseeInfo.licenseeProject, 
				FxAnalysisLicenseeInfoMaxLen,
				fonixLicenseeInfo.licenseeProject,
				FxAnalysisLicenseeInfoMaxLen);
		licenseeInfo.licenseeProject[FxAnalysisLicenseeInfoMaxLen] = 0;
		FxStrncpy(licenseeInfo.expirationDate, 
				FxAnalysisLicenseeInfoMaxLen,
				fonixLicenseeInfo.expirationDate,
				FxAnalysisLicenseeInfoMaxLen);
		licenseeInfo.expirationDate[FxAnalysisLicenseeInfoMaxLen] = 0;
		licenseeInfo.isExpired = fonixLicenseeInfo.expirationDate[127] == 1;

		if( retval == FxAnalysisError_LicenseExpired )
		{
			// Force the library to expired.
			licenseeInfo.isExpired = FxTrue;
		}

		if( retval != FxAnalysisError_LicenseExpired )
		{
			_ShutdownAnalysis();
		}

		return FxAnalysisError_None;
	}

	return FxAnalysisError_DataNotFound;
}

FxAnalysisLanguageInfo::FxAnalysisLanguageInfo(
							FxInt32        iLanguageCode,
							const FxChar*  iLanguageName,
							const FxWChar* iNativeLanguageName,
							const FxChar*  iTimeoutDate,
							FxBool         iIsDefault)
	: languageCode(iLanguageCode)
	, languageName(iLanguageName)
	, nativeLanguageName(iNativeLanguageName)
	, timeoutDate(iTimeoutDate)
	, isDefault(iIsDefault)
{
}

FxAnalysisError FX_CALL FxAnalysisGetSupportedLanguages(
							const FxWString& dataPath,
							FxArray<FxAnalysisLanguageInfo>& langaugeInfos )
{
	FxAnalysisConfig config;
	config.dataPath = dataPath;
	FxAnalysisError retval = _StartupAnalysis(config);
	if( retval == FxAnalysisError_None )
	{
		langaugeInfos.Clear();

		// Query for the supported languages.
		FxInt32 numLanguages = 0;
		FxAnalysisFonixGetNumSupportedLanguages(numLanguages);
		FxAnalysisFonixLanguageInfo* fonixLanguages = 
			new FxAnalysisFonixLanguageInfo[numLanguages];
		FxAnalysisFonixGetSupportedLanguages(fonixLanguages);

		for( FxInt32 i = 0; i < numLanguages; ++i )
		{
			FxAnalysisLanguageInfo supportedLanguage(
				fonixLanguages[i].languageCode,
				fonixLanguages[i].languageName,
				fonixLanguages[i].nativeLanguageName,
				fonixLanguages[i].timeoutDate,
				fonixLanguages[i].isDefault == 1);

			langaugeInfos.PushBack(supportedLanguage);
		}

		_ShutdownAnalysis();

		// Clean up.
		delete[] fonixLanguages; fonixLanguages = NULL;
	}

	return retval;
}

FxAnalysisError FX_CALL FxAnalysisGetExpiredLanguages(
							const FxWString& dataPath,
							FxArray<FxAnalysisLanguageInfo>& langaugeInfos )
{
	FxAnalysisConfig config;
	config.dataPath = dataPath;
	FxAnalysisError retval = _StartupAnalysis(config);
	if( retval == FxAnalysisError_None )
	{
		langaugeInfos.Clear();

		// Query for the supported languages.
		FxInt32 numLanguages = 0;
		FxAnalysisFonixGetNumExpiredLanguages(numLanguages);
		FxAnalysisFonixLanguageInfo* fonixLanguages = 
			new FxAnalysisFonixLanguageInfo[numLanguages];
		FxAnalysisFonixGetExpiredLanguages(fonixLanguages);

		for( FxInt32 i = 0; i < numLanguages; ++i )
		{
			FxAnalysisLanguageInfo supportedLanguage(
				fonixLanguages[i].languageCode,
				fonixLanguages[i].languageName,
				fonixLanguages[i].nativeLanguageName,
				fonixLanguages[i].timeoutDate,
				fonixLanguages[i].isDefault == 1);

			langaugeInfos.PushBack(supportedLanguage);
		}

		_ShutdownAnalysis();

		// Clean up.
		delete[] fonixLanguages; fonixLanguages = NULL;
	}

	return retval;
}

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------

static const FxChar audioMinStr[]           = "audiomin";
static const FxChar audioMaxStr[]           = "audiomax";
static const FxChar minPhonemeDurationStr[] = "minphonemeduration";
static const FxChar speechDetectionStr[]    = "speechdetection";

FxAnalysisError FX_CALL FxAnalysisGetParameter(
							const FxString& parameterName,
							FxReal&         parameterValue)
{
	FxString lowercaseName = parameterName.ToLower();
	FxBool   successful    = FxFalse;
	if( lowercaseName == audioMinStr )
	{
		FxAnalysisFonixGetAudioMinimum(parameterValue);
		successful = FxTrue;
	}
	else if( lowercaseName == audioMaxStr )
	{
		FxAnalysisFonixGetAudioMaximum(parameterValue);
		successful = FxTrue;
	}
	else if( lowercaseName == minPhonemeDurationStr )
	{
		FxAnalysisFonixGetMinimumPhonemeLength(parameterValue);
		successful = FxTrue;
	}
	else if( lowercaseName == speechDetectionStr )
	{
		parameterValue = g_speechDetectionEnabled ? 1.0f : 0.0f;
		successful = FxTrue;
	}

	return successful ? 
			FxAnalysisError_None : 
			FxAnalysisError_InvalidParameterName;
}

FxAnalysisError FX_CALL FxAnalysisSetParameter(
							const FxString& parameterName,
							FxReal          parameterValue)
{
	FxString lowercaseName = parameterName.ToLower();
	FxBool   successful    = FxFalse;
	if( lowercaseName == audioMinStr )
	{
		FxAnalysisFonixSetAudioMinimum(parameterValue);
		successful = FxTrue;
	}
	else if( lowercaseName == audioMaxStr )
	{
		FxAnalysisFonixSetAudioMaximum(parameterValue);
		successful = FxTrue;
	}
	else if( lowercaseName == speechDetectionStr )
	{
		if( FxRealEquality(0.0f, parameterValue) )
		{
			g_speechDetectionEnabled = FxFalse;
		}
		else
		{
			g_speechDetectionEnabled = FxTrue;
		}
		successful = FxTrue;
	}

	return successful ? 
			FxAnalysisError_None : 
			FxAnalysisError_InvalidParameterName;
}

//------------------------------------------------------------------------------
// Helper function definitions
//------------------------------------------------------------------------------

void FX_CALL _LogOutput(const wchar_t* msg, FxAnalysisFonixLogChannel chan)
{
	FxString converter(msg);
	switch(chan)
	{
	case FAFLC_Error:
		FxAnalysisLogDevError(converter.GetData());
		break;
	case FAFLC_Warn:
		FxAnalysisLogDevWarn(converter.GetData());
		break;
	default:
	case FAFLC_Trace:
		FxAnalysisLogDevTrace(converter.GetData());
		break;
	}
}

FxBool FX_CALL _IsLanguageSupported(
							const FxAnalysisConfig& analysisConfig,
							FxInt32&				languageCode )
{
	FxBool retval = FxFalse;

	if( _StartupAnalysis(analysisConfig) == FxAnalysisError_None )
	{
		// Query for the supported languages.
		FxInt32 numLanguages = 0;
		FxAnalysisFonixGetNumSupportedLanguages(numLanguages);
		FxAnalysisFonixLanguageInfo* languageInfos = 
			new FxAnalysisFonixLanguageInfo[numLanguages];
		FxAnalysisFonixGetSupportedLanguages(languageInfos);

		// Check if we have a language whose name matches what the user requested.
		for( FxInt32 i = 0; i < numLanguages; ++i )
		{
			if( FxStrcmp(languageInfos[i].languageName, 
						analysisConfig.language.GetData()) == 0 )
			{
				// If so, fill out the language code.
				languageCode = languageInfos[i].languageCode;
				retval = FxTrue;
				break;
			}

			// We have changed the names of American English and British English
			// to USEnglish and UKEnglish as of version 1.7. Insert a couple of
			// special checks to see if the user is attempting to reanalyze
			// range from a file saved in a prior version of FaceFX Studio.
			// @todo remove this in a few versions.
			if( (FxStrcmp(analysisConfig.language.GetData(), "American English") == 0 &&
				 FxStrcmp(languageInfos[i].languageName, "USEnglish") == 0 ) ||
				(FxStrcmp(analysisConfig.language.GetData(), "British English") == 0 &&
				 FxStrcmp(languageInfos[i].languageName, "UKEnglish") == 0 ) )
			{
				// If so, fill out the language code.
				languageCode = languageInfos[i].languageCode;
				retval = FxTrue;
				break;
			}
		}

		// Clean up and return our result.
		_ShutdownAnalysis();
		delete[] languageInfos; languageInfos = NULL;
	}
	return retval;
}

FxAnalysisError FX_CALL _StartupAnalysis(
							const FxAnalysisConfig& analysisConfig )
{
// 	FxWString message("Starting analysis with data directory: ");
// 	message += analysisConfig.dataPath;
// 	FxAnalysisLogTrace(message.GetCstr());

	// Attempt to start up the analysis.
	FxAnalysisFonixError err = 
		FxAnalysisFonixStartup(analysisConfig.dataPath.GetData());

	// Translate the error code and log the error.
	FxAnalysisError retval = FxAnalysisError_None;
	if( err == FX_ANALYSIS_FONIX_LICENSE_EXPIRED )
	{
		FxAnalysisLogError("Startup failed: The analysis library license has expired.");
		retval = FxAnalysisError_LicenseExpired;
	}
	else if( err == FX_ANALYSIS_FONIX_STARTUP_FAILED )
	{
		FxAnalysisLogError("Startup failed: The analysis data could not be found.");
		retval = FxAnalysisError_DataNotFound;
	}
	else if( err == FX_ANALYSIS_FONIX_NULL_PARAMETER )
	{
		FxAnalysisLogError("Startup failed: An important parameter was NULL.");
		retval = FxAnalysisError_NullParameter;
	}

	return retval;
}

// Send the audio to the analysis library.
FxAnalysisError FX_CALL _ProcessAudio(
							 const FxAnalysisAudio& audioInformation,
							 const FxWString&		text,
							 const FxInt32          languageCode,
							 FxAnalysisResults&		analysisResults )
{
	FxAnalysisError retval = FxAnalysisError_None;

	// Check what the maximum audio duration is that the analysis can analyze
	// successfully.  If the audio is longer than the max audio size, it will
	// need to be chunked into the analysis engine without text.
	FxReal maxAudioDuration = 90.f;
	FxAnalysisFonixGetAudioMaximum(maxAudioDuration);
	FxReal audioDuration =  audioInformation.numSamples / 
		static_cast<FxReal>(audioInformation.sampleRate);

	if( audioDuration < maxAudioDuration )
	{
		retval = __AnalyzeWithText(audioInformation, text, languageCode, 
			                       analysisResults);
	}
	else
	{
		retval = __ChunkAnalyze(audioInformation, languageCode, 
			                    analysisResults);
	}

	return retval;
}

FxAnalysisError FX_CALL _ShutdownAnalysis()
{
	// Attempt to shut down the analysis.
	FxAnalysisFonixError err = FxAnalysisFonixShutdown();
	
	// Translate the error code log the error.
	FxAnalysisError retval = FxAnalysisError_None;
	if( err == FX_ANALYSIS_FONIX_SHUTDOWN_FAILED )
	{
		FxAnalysisLogWarning("Internal error: Fonix shutdown failed.");
		retval = FxAnalysisError_InternalError;
	}

	return retval;
}

FxAnalysisError FX_CALL _ProofAudio(
							const FxAnalysisAudio& audioInformation)
{
	if( audioInformation.bitsPerSample != 16 )
	{
		FxAnalysisLogError("The bits per sample must be 16");
		return FxAnalysisError_InvalidBitsPerSample;
	}
	if( !(audioInformation.sampleRate == 16000 ||
		  audioInformation.sampleRate == 8000) )
	{
		FxAnalysisLogError("The sample rate must be 16000 or 8000");
		return FxAnalysisError_InvalidSampleRate;
	}
	if( audioInformation.channelCount != 1 )
	{
		FxAnalysisLogError("The channel count must be mono");
		//@todo just take one channel of the audio?
		return FxAnalysisError_InvalidChannelCount;
	}
	if( audioInformation.pAudio == NULL )
	{
		FxAnalysisLogError("The audio pointer is NULL");
		return FxAnalysisError_NullParameter;
	}
	return FxAnalysisError_None;
}

FxAnalysisError FX_CALL _RunCoarticulation(
							FxAnalysisConfig& analysisConfig,
							FxAnalysisResults& analysisResults)
{
	// Set up the coarticulation.
	FxCoarticulationGeneric coarticulation;
	coarticulation.SetConfig(*analysisConfig.pCoarticulationConfig);

	// Add the phonemes to the coarticulation.
	FxSize numPhones = analysisResults.phonemeTimes.Length();
	coarticulation.BeginPhonemeList(numPhones);
	for( FxSize i = 0; i < numPhones; ++i )
	{
		coarticulation.AppendPhoneme(
			analysisResults.phonemeTimes[i].phoneEnum,
			analysisResults.phonemeTimes[i].phoneStart,
			analysisResults.phonemeTimes[i].phoneEnd);
	}
	coarticulation.EndPhonemeList();

	// Add the mapping to the coarticulation
	const FxPhonemeMap* pPhonemeMap = analysisConfig.pPhonemeMap;
	FxSize numMappingEntries = pPhonemeMap->GetNumMappingEntries();
	coarticulation.BeginMapping();
	for( FxSize i = 0; i < numMappingEntries; ++i )
	{
		const FxPhonToNameMap& mapEntry = pPhonemeMap->GetMappingEntry(i);
		coarticulation.AddMappingEntry(mapEntry.phoneme, 
									   mapEntry.name, 
									   mapEntry.amount);
	}
	coarticulation.EndMapping();

	// Run the coarticulation.
	coarticulation.Process();

	// Add the resulting curves to the animation.
	FxSize numCurves = coarticulation.GetNumCurves();
	for( FxSize i = 0; i < numCurves; ++i )
	{
		analysisResults.animation.AddAnimCurve(coarticulation.GetCurve(i));
	}

	return FxAnalysisError_None;
}

FxAnalysisError FX_CALL _RunSpeechGestures(
							const FxAnalysisAudio& audioInformation,
							FxAnalysisConfig&      analysisConfig,
							FxAnalysisResults&     analysisResults )
{
	FxDReal* audioBuffer = _CreateSampleBuffer(audioInformation);

	// Set up the speech gestures.
	FxSpeechGestures speechGestures;
	speechGestures.SetConfig(*analysisConfig.pGestureConfig);

	speechGestures.Initialize(
		audioBuffer,
		audioInformation.numSamples,
		audioInformation.sampleRate);
	
	// Export the phoneme list into the format the speech gestures expects.
	FxSize numPhones = analysisResults.phonemeTimes.Length();
	FxPhonemeList phonList;
	for( FxSize i = 0; i < numPhones; ++i )
	{
		phonList.AppendPhoneme(
			analysisResults.phonemeTimes[i].phoneEnum,
			analysisResults.phonemeTimes[i].phoneStart,
			analysisResults.phonemeTimes[i].phoneEnd);
	}

	// Align the speech gestures to the phoneme list.
	speechGestures.Align(&phonList, analysisConfig.randomSeed);

	// Add the resulting curves to the animation.
	for( FxSize i = GT_First; i <= GT_Last; ++i )
	{
		analysisResults.animation.AddAnimCurve(
			speechGestures.GetTrack(static_cast<FxGestureTrack>(i)));
	}

	delete[] audioBuffer; audioBuffer = NULL;
	return FxAnalysisError_None;
}

FxDReal* FX_CALL _CreateSampleBuffer(
							const FxAnalysisAudio& audioInformation )
{
	FxDReal* retval = NULL;
	if( audioInformation.numSamples > 0 && 
		audioInformation.pAudio != NULL )
	{
		retval = new FxDReal[audioInformation.numSamples];

		// Convert and store all the samples.
		const FxByte* pAudio = audioInformation.pAudio;
		for( FxUInt32 i = 0; i < audioInformation.numSamples; ++i )
		{
			// If it has multiple channels, this will ensure only the left
			// channel is read.
			FxUInt32 index = i * audioInformation.channelCount;

			// Get the sample value as a floating point value.
			FxDReal sampleValue = 0.0;
			if( audioInformation.bitsPerSample == 8 )
			{
				sampleValue =
					reinterpret_cast<const FxUInt8*>(pAudio)[index];
				// Normalize the 8 bit sample.
				sampleValue -= -SCHAR_MIN;
			}
			else if( audioInformation.bitsPerSample == 16 )
			{
				sampleValue =
					reinterpret_cast<const FxInt16*>(pAudio)[index];
			}

			// Store the sample value.
			retval[i] = sampleValue;
		}
	}
	return retval;
}

FxAnalysisError FX_CALL _ProcessTextTags(
							const FxWString&   text,
							FxAnalysisResults& analysisResults )
{
	if( text.Length() == 0 )
	{
		return FxAnalysisError_None;
	}

	// If there are no words, we can still run the text tag parser and
	// it will return word indices of 0.  Before using the results however,
	// Make sure there is at least one word at index 0 with start and end time 
	// equal to zero.
	FxArray<FxWordTimeInfo> emptywordTimes;
	FxArray<FxWordTimeInfo>& wordTimes = emptywordTimes;
	FxUnicodeTextTagParser parser(text);
	parser.Parse(analysisResults.wordTimes);
	FxAnim& anim = analysisResults.animation;
	if(analysisResults.wordTimes.Length() == 0)
	{
		wordTimes.PushBack(	FxWordTimeInfo(FxWString(), 0.0f, 0.0f));													
	}
	else
	{
		wordTimes = analysisResults.wordTimes;
	}
	

	// Process the text tags.
	FxSize numTextTags = parser.GetNumTextTags();
	for( FxSize currentTag = 0; currentTag < numTextTags; ++currentTag )
	{
		const FxUnicodeTextTagInfo& tag = parser.GetTextTag(currentTag);

		FxSize curveIndex = anim.FindAnimCurve(tag.GetCurveName().GetData());
		if( curveIndex == FxInvalidIndex )
		{
			// Add a new curve.
			FxAnimCurve newCurve;
			newCurve.SetName(tag.GetCurveName().GetData());
			anim.AddAnimCurve(newCurve);
			curveIndex = anim.FindAnimCurve(tag.GetCurveName().GetData());
			FxAssert( curveIndex != FxInvalidIndex );
		}
		// Modify an existing curve.
		FxAnimCurve& animCurve = anim.GetAnimCurveM(curveIndex);

		// Line up the keys with word timings.
		FxReal v2Time = wordTimes[tag.GetStartWord()].wordStart + 
						tag.GetTimeShift();
		FxReal v3Time = wordTimes[tag.GetEndWord()].wordEnd +
						tag.GetTimeShift();

		// Handle quadruplets.
		if( tag.GetCurveType() == FxUnicodeTextTagInfo::CT_Quadruplet )
		{
			FxAnimKey v1(v2Time - tag.GetEaseIn(), tag.GetV1());
			animCurve.InsertKey(v1);

			if( tag.GetDuration() != FxInvalidValue )
			{
				v3Time = v2Time + tag.GetDuration();
			}
			FxAnimKey v2(v2Time, tag.GetV2());
			animCurve.InsertKey(v2);
			FxAnimKey v3(v3Time, tag.GetV3());
			animCurve.InsertKey(v3);

			if( !FxRealEquality(tag.GetV4(), tag.GetV3()) )
			{
				FxAnimKey v4(v3Time + tag.GetEaseOut(), tag.GetV4());
				animCurve.InsertKey(v4);
			}
		}
		else
		{
			// Handle all triplets.
			FxReal centerTime = 0.0f;
			if( tag.GetCurveType() == 
				FxUnicodeTextTagInfo::CT_LeadingTriplet )
			{
				centerTime = v2Time;
			}
			else if( tag.GetCurveType() == 
				FxUnicodeTextTagInfo::CT_CenterTriplet )
			{
				centerTime = v2Time + ((v3Time - v2Time) * 0.5f);
			}
			else if( tag.GetCurveType() == 
				FxUnicodeTextTagInfo::CT_TrailingTriplet )
			{
				centerTime = v3Time;
			}

			FxAnimKey v1(v2Time - tag.GetEaseIn(), tag.GetV1());
			animCurve.InsertKey(v1);

			FxAnimKey centerKey(centerTime, tag.GetV2());
			animCurve.InsertKey(centerKey);

			FxAnimKey v3(v3Time + tag.GetEaseOut(), tag.GetV3());
			animCurve.InsertKey(v3);
		}
	}

	return FxAnalysisError_None;
}

FxAnalysisError FX_CALL __AnalyzeWithText(
							const FxAnalysisAudio& audioInformation,
							const FxWString&       text,
							const FxInt32          languageCode,
							FxAnalysisResults&     analysisResults )
{
	FxBool usedText = FxTrue;
	FxAnalysisFonixError err = FxAnalysisFonixAnalyze(
		audioInformation.bitsPerSample,
		audioInformation.sampleRate,
		audioInformation.channelCount,
		audioInformation.numSamples,
		reinterpret_cast<const FxChar*>(audioInformation.pAudio),
		text.Length(),
		text.GetData(),
		languageCode,
		g_speechDetectionEnabled );

	// Ensure we're not htting the case where we should be chunking audio here.
	FxAssert(err != FX_ANALYSIS_FONIX_TOO_MUCH_AUDIO);
	if( err == FX_ANALYSIS_FONIX_TOO_MUCH_AUDIO )
	{
		FxAnalysisLogError("Internal error: Too much audio was passed in at "
						   "one time. If you encounter this error, please "
						   "notify support@oc3ent.com");
		return FxAnalysisError_InternalError;
	}

	// If we failed in the first attempt, retry with no text and speech 
	// detection disabled.
	if( err != FX_ANALYSIS_FONIX_NOERROR )
	{
		FxAnalysisLogWarning("First analysis attempt failed. Trying again with "
							 "no text and speech detection disabled.");
		usedText = FxFalse;
		err = FxAnalysisFonixAnalyze(
			audioInformation.bitsPerSample,
			audioInformation.sampleRate,
			audioInformation.channelCount,
			audioInformation.numSamples,
			reinterpret_cast<const FxChar*>(audioInformation.pAudio),
			0,
			NULL,
			languageCode,
			FxFalse );
	}

	CHECK_FXAL_ERROR_NOTIFY_SUCCESS(err, "Analysis was successful.");

	if( text.Length() && usedText )
	{
		FxWString message(FxW("Used text: "));
		message += text;
		FxAnalysisLogTrace(message.GetCstr()); // @todo make unicode compatible.
	}

	// Attempt to give a passable animation for a very short audio file.
	if( err == FX_ANALYSIS_FONIX_NOT_ENOUGH_AUDIO )
	{
		FxAnalysisLogWarning("Attempting to recover from previous error: "
			                 "Reticulating splines to generate placeholder "
							 "animation.");
		FxReal audioDuration = audioInformation.numSamples /
							   static_cast<FxReal>(audioInformation.sampleRate);
		analysisResults.phonemeTimes.PushBack(
			FxPhoneTimeInfo(PHON_EH, 0.0f, audioDuration));
		if( text.Length() )
		{
			analysisResults.wordTimes.PushBack(
				FxWordTimeInfo(text, 0.0f, audioDuration));
		}
		return FxAnalysisError_None;
	}

	// Get the phonemes from the analysis engine.
	FxInt32 numPhonemes = 0;
	FxInt32 phonEnum    = 0;
	FxDReal phonStart   = 0.0;
	FxDReal phonEnd     = 0.0;
	err = FxAnalysisFonixGetNumPhonemes(numPhonemes);
	CHECK_FXAL_ERROR(err);
	for( FxInt32 i = 0; i < numPhonemes; ++i )
	{
		err = FxAnalysisFonixGetPhonemeInfo(i, phonEnum, phonStart, phonEnd);
		CHECK_FXAL_ERROR(err);
		FxPhoneTimeInfo newPhone(static_cast<FxPhoneme>(phonEnum), 
			                     static_cast<FxReal>(phonStart), 
								 static_cast<FxReal>(phonEnd));
		analysisResults.phonemeTimes.PushBack(newPhone);
	}

	// Get the words from the analysis engine.
	FxInt32       numWords          = 0;
	const FxInt32 wordLength        = 128;
	FxWChar       word[wordLength]  = {0};
	FxDReal       wordStart         = 0.0;
	FxDReal       wordEnd           = 0.0;
	err = FxAnalysisFonixGetNumWords(numWords);
	CHECK_FXAL_ERROR(err);
	for( FxInt32 i = 0; i < numWords; ++i )
	{
		err = FxAnalysisFonixGetWordInfo(i, wordLength, word, 
			                             wordStart, wordEnd);
		CHECK_FXAL_ERROR(err);
		FxWordTimeInfo newWord(word, 
							   static_cast<FxReal>(wordStart), 
							   static_cast<FxReal>(wordEnd));
		analysisResults.wordTimes.PushBack(newWord);
	}

	return FxAnalysisError_None;
}

FxAnalysisError FX_CALL __ChunkAnalyze(
							const FxAnalysisAudio& audioInformation,
							const FxInt32          languageCode,
							FxAnalysisResults&     analysisResults )
{
	FxAnalysisLogTrace("Audio too long to analyze in one pass. The audio will "
					   "be broken into chunks and analyzed without text.");

	// Determine the appropriate chunk length.  The limit for Fonix is 90 
	// seconds, but to be safe we first attempt 60 second chunks so that any 
	// leftover audio should never be greater than 90 seconds.  Taking 
	// two-thirds of the audio max (defaulted to 90 seconds) gives 60 seconds 
	// as the result.  Taking two-thirds of the audio max in this way should 
	// allow for arbitrary audio max values with the same basic algorithm.
	FxReal audioMax = 90.0f;
	FxAnalysisFonixGetAudioMaximum(audioMax);
	FxDReal chunkSize     = static_cast<FxReal>(audioMax) * (2.0 / 3.0);
	FxDReal audioDuration = audioInformation.numSamples / 
							static_cast<FxDReal>(audioInformation.sampleRate);
	FxSize  numChunks     = static_cast<FxSize>(audioDuration / chunkSize);
	FxDReal leftOverAudio = audioDuration - (numChunks * chunkSize);
	numChunks += (0.0 == leftOverAudio) ? 0 : 1;

	//FxMsg( "audioDuration: %f  leftOverAudio: %f  chunkSize: %fs  numChunks: %d",
	//	audioDuration, leftOverAudio, chunkSize, numChunks );

	// More often than not straight 60 second chunks should work, but if they 
	// don't we must converge on a chunk size that will work.
	FxReal audioMin = 0.5f;
	FxAnalysisFonixGetAudioMinimum(audioMin);
	if( 0.0 != leftOverAudio && 
		leftOverAudio < static_cast<FxReal>(audioMin) )
	{
		FxDReal slack = leftOverAudio * 10.0;
		while( slack <= static_cast<FxReal>(audioMin) )
		{
			slack *= 10.0;
		}
		chunkSize -= slack;
		numChunks = static_cast<FxSize>(audioDuration / chunkSize);
		leftOverAudio = audioDuration - (numChunks * chunkSize);
		numChunks += (0.0 == leftOverAudio) ? 0 : 1;
		//FxMsg( "convergence:  audioDuration: %f  leftOverAudio: %f  chunkSize: %fs  numChunks: %d",
		//	audioDuration, leftOverAudio, chunkSize, numChunks );
	}

	// Chunk the audio through the analysis engine.
	FxDReal chunkStart     = 0.0;
	FxDReal chunkEnd       = chunkSize;
	FxAnalysisError retval = FxAnalysisError_None;
	for( FxSize i = 0; i < numChunks; ++i )
	{
		FxString message("Analyzing chunk ");
		message << i + 1 << " of " << numChunks << ": [" 
				<< static_cast<FxReal>(chunkStart) << ", " 
				<< static_cast<FxReal>(chunkEnd) << "]";
		FxAnalysisLogTrace(message.GetData());

		retval = ___AnalyzeSingleChunk(audioInformation, 
									   languageCode, 
									   chunkStart,
									   chunkEnd,
									   analysisResults);

		if( retval != FxAnalysisError_None )
		{
			FxAnalysisLogError("Analysis failed: the previous chunk could not "
							   "be analyzed.");
			break;
		}

		// Update the chunk information.
		chunkStart = chunkEnd;
		chunkEnd  += chunkSize;
		chunkEnd   = FxClamp(0.0, chunkEnd, audioDuration);
	}

	return retval;
}

FxAnalysisError FX_CALL ___AnalyzeSingleChunk(
							const FxAnalysisAudio& audioInformation,
							const FxInt32          languageCode,
							const FxDReal          chunkStart,
							const FxDReal          chunkEnd,
							FxAnalysisResults&     analysisResults )
{
	FxReal duration = static_cast<FxReal>(chunkEnd - chunkStart);
	FxReal audioMax = 90.0f;
	FxAnalysisFonixGetAudioMaximum(audioMax);
	if( duration > audioMax )
	{
		FxAnalysisLogError("Internal error: An audio chunk was of greater "
						   "duration than the maximum allowed chunk size.");
		return FxAnalysisError_InternalError;
	}

	// Calculate some necessary values.
	FxSize numSamples  = static_cast<FxSize>((chunkEnd - chunkStart) * 
						 audioInformation.sampleRate);
	FxSize startSample = static_cast<FxSize>(chunkStart * 
		                 audioInformation.sampleRate);
	FxSize startOffset = (startSample * audioInformation.bitsPerSample) / 
					     FX_CHAR_BIT;

	FxAnalysisFonixError err = FxAnalysisFonixAnalyze(
		audioInformation.bitsPerSample,
		audioInformation.sampleRate,
		audioInformation.channelCount,
		numSamples,
		reinterpret_cast<const FxChar*>(audioInformation.pAudio + startOffset),
		0,
		NULL,
		languageCode,
		g_speechDetectionEnabled );
	CHECK_FXAL_ERROR_NOTIFY_SUCCESS(err, "Analysis was successful.");

	// Get the phonemes from the analysis engine.
	FxInt32 numPhonemes = 0;
	FxInt32 phonEnum    = 0;
	FxDReal phonStart   = 0.0;
	FxDReal phonEnd     = 0.0;
	err = FxAnalysisFonixGetNumPhonemes(numPhonemes);
	CHECK_FXAL_ERROR(err);
	for( FxInt32 i = 0; i < numPhonemes; ++i )
	{
		err = FxAnalysisFonixGetPhonemeInfo(i, phonEnum, phonStart, phonEnd);
		CHECK_FXAL_ERROR(err);
		FxPhoneTimeInfo newPhone(static_cast<FxPhoneme>(phonEnum), 
								 static_cast<FxReal>(phonStart + chunkStart), 
								 static_cast<FxReal>(phonEnd + chunkStart));
		analysisResults.phonemeTimes.PushBack(newPhone);
	}

	// Get the words from the  analysis engine.
	FxInt32       numWords          = 0;
	const FxInt32 wordLength        = 128;
	FxWChar       word[wordLength]  = {0};
	FxDReal       wordStart         = 0.0;
	FxDReal       wordEnd           = 0.0;
	err = FxAnalysisFonixGetNumWords(numWords);
	CHECK_FXAL_ERROR(err);
	for( FxInt32 i = 0; i < numWords; ++i )
	{
		err = FxAnalysisFonixGetWordInfo(i, wordLength, word, 
										 wordStart, wordEnd);
		CHECK_FXAL_ERROR(err);
		FxWordTimeInfo newWord(word, 
							   static_cast<FxReal>(wordStart + chunkStart), 
							   static_cast<FxReal>(wordEnd + chunkStart));
		analysisResults.wordTimes.PushBack(newWord);
	}

	return FxAnalysisError_None;
}

} // namespace Face

} // namespace OC3Ent
