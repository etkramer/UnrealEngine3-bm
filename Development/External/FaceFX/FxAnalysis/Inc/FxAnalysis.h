//------------------------------------------------------------------------------
// A component for generating FaceFX animations outside of FaceFX Studio.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnalysis_H__
#define FxAnalysis_H__

#include "FxPlatform.h"
#include "FxPhonemeEnum.h"
#include "FxPhonemeMap.h"
#include "FxArray.h"
#include "FxString.h"
#include "FxAnim.h"

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------
class FxCoarticulationConfig;
class FxGestureConfig;

//------------------------------------------------------------------------------
/// \defgroup supportStructs Supporting Structures
/// \brief Structures used to pass into the analysis functions.
///
/// @{
//------------------------------------------------------------------------------

/// The audio information to pass to the analysis.
struct FxAnalysisAudio
{
	/// Default constructor.
	/// \param ipAudio The initial value for \a pAudio
	/// \param iNumSamples The initial value for \a numSamples.
	/// \param iSampleRate The initial value for \a sampleRate.
	/// \param iBitsPerSample The initial value for \a bitsPerSample.
	/// \param iChannelCount The initial value for \a channelCount.
	FxAnalysisAudio(const FxByte* ipAudio        = NULL,
					FxSize        iNumSamples    = 0,
					FxSize        iSampleRate    = 16000,
					FxSize        iBitsPerSample = 16,
					FxSize        iChannelCount  = 1);
	/// A pointer to the audio.
	const FxByte* pAudio;
	/// The number of samples in the audio.
	FxSize  numSamples;
	/// The sample rate of the audio in Hz.
	FxSize  sampleRate;
	/// The bits per sample of the audio.
	FxSize  bitsPerSample;
	/// The number of channels in the audio.
	FxSize  channelCount;
};

/// The analysis configuration information.
struct FxAnalysisConfig
{
	/// Default constructor.
	FxAnalysisConfig();
	/// The path to the analysis data.
	FxWString dataPath;
	/// The phoneme to track mapping to use when analyzing.
	const FxPhonemeMap* pPhonemeMap;
	/// The name of the language to use when analyzing.
	FxString language;
	/// The coarticulation configuration to use when generating speech curves.
	const FxCoarticulationConfig* pCoarticulationConfig;
	/// The gesture configuration to use when generating speech gestures.
	const FxGestureConfig* pGestureConfig;
	/// Whether to generate speech gestures.
	FxBool generateSpeechGestures;
	/// The random seed to use for the animation.
	FxInt32 randomSeed;
};

/// The timing information for a single phoneme.
struct FxPhoneTimeInfo
{
	/// Default constructor.
	FxPhoneTimeInfo(FxPhoneme iPhoneEnum  = PHON_SIL,
					FxReal    iPhoneStart = 0.0f,
					FxReal    iPhoneEnd   = 0.0f);
	/// The phoneme enumeration.
	FxPhoneme phoneEnum;
	/// The phoneme start time.
	FxReal phoneStart;
	/// The phoneme end time.
	FxReal phoneEnd;
};

/// The timing information for a single word.
struct FxWordTimeInfo
{
	/// Default constructor.
	FxWordTimeInfo(const FxWString& iWordText  = FxWString(""),
				   FxReal           iWordStart = 0.0f,
				   FxReal           iWordEnd   = 0.0f);
	/// The word text.
	FxWString wordText;
	/// The word start time.
	FxReal wordStart;
	/// The word end time.
	FxReal wordEnd;
};

/// The analysis results.
struct FxAnalysisResults
{
	/// The phoneme times, in chronological order.
	FxArray<FxPhoneTimeInfo> phonemeTimes;
	/// The word times, in chronological order.
	FxArray<FxWordTimeInfo> wordTimes;
	/// The resulting animation.
	FxAnim animation;
};

/// @}

//------------------------------------------------------------------------------
/// \defgroup error Error Handling
/// \brief Error handling
/// 
/// @{ 
//------------------------------------------------------------------------------

/// The error codes that may be returned from the functions.
enum FxAnalysisError
{
	/// No error.
	FxAnalysisError_None = 0,
	/// The license to the analysis has expired.
	FxAnalysisError_LicenseExpired,
	/// A required parameter was NULL.
	FxAnalysisError_NullParameter,
	/// The bits per sample is unsupported.
	FxAnalysisError_InvalidBitsPerSample,
	/// The sample rate is unsupported.
	FxAnalysisError_InvalidSampleRate,
	/// The channel count is unsupported.
	FxAnalysisError_InvalidChannelCount,
	/// The language is unsupported
	FxAnalysisError_InvalidLanguage,
	/// The data was not found.
	FxAnalysisError_DataNotFound,
	/// A parameter of that name was not found.
	FxAnalysisError_InvalidParameterName,
	/// An internal error occurred.
	FxAnalysisError_InternalError,
};

/// Returns a human-readable string about the error code that was generated.
/// \param[in] errCode The ::FxAnalysisError code to get information about.
/// \return A string with information about \a errCode.
FxString FxAnalysisGetErrorInformation(FxAnalysisError errCode);

/// @}

//------------------------------------------------------------------------------
/// \defgroup analysis Audio and Phonetic Analysis
/// \brief Functions to analyze audio and phoneme lists to create animations
///
/// @{
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// One-step analysis
//------------------------------------------------------------------------------

/// Analyzes the audio and returns the complete result set.
/// \param[in] audioInformation A pointer to the audio and information about
///   the audio format.
/// \param[in] text The text to use when analyzing the audio. Use an empty 
///   string to signify text-less analysis.
/// \param[in] analysisConfig The configuration for the analysis engine to use
///   when generating the resulting animation.
/// \param[out] analysisResults The results of the analysis process.
/// \return
///	  - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalyze( 
							const FxAnalysisAudio&  audioInformation,
							const FxWString&        text,
							FxAnalysisConfig&       analysisConfig,
							FxAnalysisResults&      analysisResults );

//------------------------------------------------------------------------------
// Two-step analysis
//------------------------------------------------------------------------------

/// Analyzes the audio and returns the phoneme and word timing information.
/// \param[in] audioInformation A pointer to the audio and information about
///   the audio format.
/// \param[in] text The text to use when analyzing the audio. Use an empty 
///   string to signify text-less analysis.
/// \param[in] analysisConfig The configuration for the analysis engine to use
///   when generating the resulting animation.
/// \param[out] analysisResults The results of the analysis process, of which
///   only \a phonemeTimes and \a wordTimes are valid.  The animation is not
///   created by this stage of the process.
/// \return
///	  - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalyzeAudio( 
							const FxAnalysisAudio&  audioInformation,
							const FxWString&        text,
							FxAnalysisConfig&       analysisConfig,
							FxAnalysisResults&      analysisResults );

/// Analyzes the phoneme and word times and returns an animation.
/// \param[in] audioInformation A pointer to the audio and information about
///   the audio format.
/// \param[in] analysisConfig The configuration for the analysis engine to use
///   when generating the resulting animation.
/// \param[in] text The text to use when analyzing the audio. Use an empty 
///   string to signify text-less analysis.
/// \param[in,out] analysisResults The phoneme and word times must be filled in
///   before calling this function.  The animation is created by this stage 
///   of the process.
/// \return
///	  - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalyzePhonemes( 
							const FxAnalysisAudio& audioInformation,
							const FxWString&       text,
							FxAnalysisConfig&      analysisConfig,
							FxAnalysisResults&     analysisResults );

/// @}

//------------------------------------------------------------------------------
///\defgroup logging Logging
/// \brief Interface for receiving logging information from the analysis 
/// library.
///
/// @{
//------------------------------------------------------------------------------

/// The channels of output from the analysis library.
enum FxAnalysisLogChannel
{
	FxAnalysisChannel_Error    = 1,
	FxAnalysisChannel_Warning  = 2,
	FxAnalysisChannel_Trace    = 4,
	FxAnalysisChannel_DevError = 8,
	FxAnalysisChannel_DevWarn  = 16,
	FxAnalysisChannel_DevTrace = 32

};

/// The logging output callback function signature.
typedef void (*FxAnalysisLogOutputFunc)(const FxChar*, FxAnalysisLogChannel);

/// Sets the log output callback function pointer.
/// \param[in] pLogOutputFunc A pointer to the logging output callback.
/// \return
///   - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalysisSetLogOutputFunction(
							FxAnalysisLogOutputFunc pLogOutputFunc );

/// @}

//------------------------------------------------------------------------------
/// \defgroup libInfo Licensee and Library Information.
/// \brief Functions for getting information about the licensee and the library.
///
/// @{
//------------------------------------------------------------------------------

/// Retrieves a version string denoting the analysis library version.
/// \param[out] versionString The version of the analysis library.
/// \return
///   - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalysisGetVersionString(
							FxString& versionString);

/// The maximum length of the fields in the licensee info struct.
const FxSize FxAnalysisLicenseeInfoMaxLen = 128;

/// Information about the licensee and library.
struct FxAnalysisLicenseeInfo
{
	// Default constructor.
	FxAnalysisLicenseeInfo();
	/// The licensee's name.
	FxChar licenseeName[FxAnalysisLicenseeInfoMaxLen];
	/// The licensee's project.
	FxChar licenseeProject[FxAnalysisLicenseeInfoMaxLen];
	/// The library expiration date
	FxChar expirationDate[FxAnalysisLicenseeInfoMaxLen];
	/// Whether the library is expired.
	FxBool isExpired;
};

/// Retrieves the licensee information.
/// \param[in] dataPath The path to the analysis data.
/// \param[out] licenseeInfo An empty licensee info struct which will be filled
///   out by the function.
/// \return
///   - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalysisGetLicenseeInfo(
							const FxWString& dataPath,
							FxAnalysisLicenseeInfo& licenseeInfo );

/// Information about a supported language.
struct FxAnalysisLanguageInfo
{
	/// Default constructor.
	FxAnalysisLanguageInfo(FxInt32        iLanguageCode       = -1,
						   const FxChar*  iLanguageName       = NULL,
						   const FxWChar* iNativeLanguageName = NULL,
						   const FxChar*  iTimeoutDate        = NULL,
						   FxBool         iIsDefault          = FxFalse);
	/// The language ID.
	FxInt32 languageCode;
	/// The language name.
	FxString languageName;
	/// The language name, in the native tongue.
	FxWString nativeLanguageName;
	/// The language's timeout date.
	FxString timeoutDate;
	/// Whether the language is the default language.
	FxBool isDefault;
};

/// Retrieves a list of the supported languages.
/// \param[in] dataPath The path to the analysis data.
/// \param[out] languageInfos Information about each language supported by the
///   analysis library.
/// \return
///   - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalysisGetSupportedLanguages(
							const FxWString& dataPath,
							FxArray<FxAnalysisLanguageInfo>& languageInfos );

/// Retrieves a list of the expired languages.
/// \param[in] dataPath The path to the analysis data.
/// \param[out] languageInfos Information about each expired language.
/// \return
///   - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalysisGetExpiredLanguages(
							const FxWString& dataPath,
							FxArray<FxAnalysisLanguageInfo>& languageInfos );

/// @}

//------------------------------------------------------------------------------
/// \defgroup config Configuration
/// \brief Advanced configuration of the analysis library, mainly used for
///   FaceFX Studio.
///
/// @{
//------------------------------------------------------------------------------

/// Gets the value of a parameter.
/// \param[in] parameterName The name of the parameter. May be one of:
///   - audioMin
///   - audioMax
///   - minPhonemeDuration
///   - speechDetection
/// \param[out] parameterValue The value of the parameter.
/// \return
///   - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalysisGetParameter(
							const FxString& parameterName,
							FxReal&         parameterValue);

/// Sets the value of a parameter.
/// \param[in] parameterName The name of the parameter. May be one of:
///   - audioMin
///   - audioMax
/// \param[in] parameterValue The value of the parameter.
/// \return
///   - FxAnalysisError_None on success.
FxAnalysisError FX_CALL FxAnalysisSetParameter(
							const FxString& parameterName,
							FxReal          parameterValue);

/// @}

} // namespace Face

} // namespace OC3Ent

#endif

//------------------------------------------------------------------------------
// This point down is purely Doxygen documentation.
//------------------------------------------------------------------------------

/// \mainpage
/// \par About FxAnalysis
///   FxAnalysis is a component that provides all of the functionality needed
///   to go from raw audio and text to a complete FaceFX animation. This 
///   involves two main steps, each of which can be used independently.
///
/// \par
///   The first step analyzes the audio and breaks it into a list of phoneme 
///   and words, along with the timing information. If you are going to do your
///   own coarticulation, you can stop at this step, since you'll have a full
///   list of the phonemes that were detected in the speech.
///
/// \par
///   The second step analyzes the phonemes and words and creates an animation
///   from the coarticulation results. If you are using a text-to-speech (TTS) 
///   engine, you can use the TTS phonetic results and bypass the first step
///   altogether, using OC3's coarticulation to generate a facial animation
///   for the TTS audio.
///
/// \par Quick Links
///   \li \ref supportStructs "Supporting Structures"
///   \li \ref error "Error Handling"
///   \li \ref logging "Logging Support"
///   \li \ref analysis "Audio and Phonetic Analysis"
///   \li \ref libInfo "Licensee and Library Information"
///   \li \ref config "Advanced Configuration"
///
/// \par Using FxAnalysis
///   To set up your project to use FxAnalysis, you need to add two paths to
///   your include paths. The path to FxSDK\\Inc and FxAnalysis\\Inc are 
///   necessary to build a project using FxAnalysis.  You will need to link to 
///   three libraries: FxSDK, FxCG and FxAnalysis.
///
/// \note FxAnalysis only provides libraries for the Debug Multithreaded DLL and
///   Multithreaded DLL code generation settings. An application wishing to link
///   to FxAnalysis will need to use one of those two code generation settings.

