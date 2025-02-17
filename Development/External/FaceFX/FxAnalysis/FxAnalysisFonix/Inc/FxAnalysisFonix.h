//------------------------------------------------------------------------------
// FaceFX Fonix based analysis core.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnalysisFonix_H__
#define FxAnalysisFonix_H__

#define FX_ANALYSIS_FONIX_API
//#define FX_ANALYSIS_FONIX_API __declspec(dllexport)

#if defined(__cplusplus)
extern "C" {
#endif

//------------------------------------------------------------------------------
// Data structures.
//------------------------------------------------------------------------------

/// Contains information about the licensee.
struct FX_ANALYSIS_FONIX_API FxAnalysisFonixLicenseeInfo
{
	char licenseeName[128];		///< Licensee name.
	char licenseeProject[128];	///< Licensee project.
	char expirationDate[128];	///< Expiration date.
};

/// Contains information about a language.
struct FX_ANALYSIS_FONIX_API FxAnalysisFonixLanguageInfo
{
	int languageCode;					///< Language code.
	char languageName[128];				///< Language name.
	wchar_t nativeLanguageName[128];	///< Language name in native tongue.
	char timeoutDate[128];				/// The timeout date, or unlimited.
	int isDefault;					    ///< 1 if the language is the default,
										///< 0 otherwise.
};

//------------------------------------------------------------------------------
// Constants.
//------------------------------------------------------------------------------

///\brief FxAnalysis error codes. Use FxAnalysisFonixGetErrorString() to return
/// text descriptions for these codes.
enum FxAnalysisFonixError
{
	/// Success (not an error).
	FX_ANALYSIS_FONIX_NOERROR = 0,
	/// FxAnalysisFonixStartup() failed.
	FX_ANALYSIS_FONIX_STARTUP_FAILED,
	/// The license for FxAnalysisFonix.dll has expired.
	FX_ANALYSIS_FONIX_LICENSE_EXPIRED,
	/// The FxAnalysisFonix data files could not be found due to an invalid 
	/// path or missing files.
	FX_ANALYSIS_FONIX_DATA_FILES_NOT_FOUND,
	/// FxAnalysisFonixStartup() has not been called.
	FX_ANALYSIS_FONIX_STARTUP_NOT_CALLED,
	/// FxAnalysisFonixShutdown() failed, either because 
	/// FxAnalysisFonixStartup() was never called or because
	/// FxAnalysisFonixShutdown() has already been called.
	FX_ANALYSIS_FONIX_SHUTDOWN_FAILED,
	/// One or more of the parameters are NULL.
	FX_ANALYSIS_FONIX_NULL_PARAMETER,
	/// One or more of the parameters is invalid.
	FX_ANALYSIS_FONIX_INVALID_PARAMETER,
	/// One or more of the indices are invalid (out of range).
	FX_ANALYSIS_FONIX_INVALID_INDEX,
	/// The audio data format is invalid (unsupported bits per sample etc).
	FX_ANALYSIS_FONIX_INVALID_AUDIO,
	/// Too much audio data (Fonix can only safely handle 90 seconds of audio 
	/// data at a time).
	FX_ANALYSIS_FONIX_TOO_MUCH_AUDIO,
	/// Not enough audio data (Fonix cannot safely handle less than 0.5 seconds 
	/// of audio data at a time).
	FX_ANALYSIS_FONIX_NOT_ENOUGH_AUDIO,
	/// Analysis failed (the audio data cannot be analyzed or an internal error
	/// occurred).
	FX_ANALYSIS_FONIX_ANALYSIS_FAILED,
	/// A serious internal error occurred.  If this ever happens, send an email
	/// to OC3 Entertainment at the following address: support@oc3ent.com
	FX_ANALYSIS_FONIX_INTERNAL_ERROR,
	/// The last error code: useful for iterating through the error codes.
	FX_ANALYSIS_FONIX_ERROR_LAST = FX_ANALYSIS_FONIX_INTERNAL_ERROR,

	/// The first error code: useful for iterating through the error codes.
	FX_ANALYSIS_FONIX_ERROR_FIRST = FX_ANALYSIS_FONIX_NOERROR
};

//------------------------------------------------------------------------------
// Function declarations.
//------------------------------------------------------------------------------

enum FxAnalysisFonixLogChannel
{
	FAFLC_Trace = 1,
	FAFLC_Warn  = 2,
	FAFLC_Error = 4,
};

/// \brief Sets the logging output callback function pointer.
/// \param pLogOutputFunction Pointer to the function to handle logging output.
/// \return \c FX_ANALYSIS_FONIX_NOERROR
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixSetLogOutputFunction( void(*iCallbackFunction)(const wchar_t*, FxAnalysisFonixLogChannel) );

//------------------------------------------------------------------------------
// Startup / Shutdown functions.
//------------------------------------------------------------------------------
///\name Startup / Shutdown Functions.
/// Functions to startup and shut down FxAnalysisFonix.
//@{
/// \brief Must be the first function called when using FxAnalysisFonix.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param iCoreDataDir \b In: Full path of folder containing FxAnalysisFonix
/// data files.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixStartup( const wchar_t* iDataFilePath );

/// \brief Should be the last function called when using FxAnalysis.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixShutdown( void );
//@}

//------------------------------------------------------------------------------
// Version Functions.
//------------------------------------------------------------------------------
///\name Version Functions.
/// Functions to return the version and license information for FxAnalysisFonix.
//@{
/// \brief Gets the FxAnalysisFonix version number.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param oVersion \b Out: the version of the Fonix libraries that this version
/// of FxAnalysisFonix was created with.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixGetVersion( int& oVersion );

/// Function to return licensee information.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param oLicenseeInfo \b Out: A licensee info struct containing name, 
/// project and expiration date.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixGetLicenseeInfo( FxAnalysisFonixLicenseeInfo& oLicenseeInfo );
//@}

//------------------------------------------------------------------------------
// Language Functions.
//------------------------------------------------------------------------------
///\name Language Functions.
/// Functions for retrieving the languages supported by this version of the DLL.
//@{
/// \brief Retrieves the number of languages this version of the DLL supports.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
/// \param oNumSupportedLanguages \b Out: The number of supported languages.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixGetNumSupportedLanguages( int& oNumSupportedLanguages );

/// \brief Retrieves the list of languages that this version of the DLL 
/// supports.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
/// \param oSupportedLanguages \b Out: A C-style array of 
/// FxAnalysisFonixLanguageInfo structures.  The array must be allocated by the
/// client before being passed in and must be large enough to hold all of the
/// supported languages.  For example, call 
/// FxAnalysisFonixGetNumSupportedLanguages() and allocate a C-style array of 
/// FxAnalysisFonixLanguageInfo structures to that size and pass that array
/// into this function.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixGetSupportedLanguages( FxAnalysisFonixLanguageInfo* oSupportedLanguages );

/// \brief Retrieves the number of languages this version of the DLL supports.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
/// \param oNumExpiredLanguages \b Out: The number of expired languages.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixGetNumExpiredLanguages( int& oNumExpiredLanguages );

/// \brief Retrieves the list of languages that this version of the DLL 
/// supports, but have expired.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
/// \param oExpiredLanguages \b Out: A C-style array of 
/// FxAnalysisFonixLanguageInfo structures.  The array must be allocated by the
/// client before being passed in and must be large enough to hold all of the
/// supported languages.  For example, call 
/// FxAnalysisFonixGetNumExpiredLanguages() and allocate a C-style array of 
/// FxAnalysisFonixLanguageInfo structures to that size and pass that array
/// into this function.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixGetExpiredLanguages( FxAnalysisFonixLanguageInfo* oExpiredLanguages );
//@}

//------------------------------------------------------------------------------
// Utility Functions.
//------------------------------------------------------------------------------
///\name Utility Functions.
/// Functions for retrieving the string associated with an 
/// \c FxAnalysisFonixError code and for registering a callback to receive the 
/// percentage of the analysis process completed.
//@{
/// \brief Convert a FxAnalysisFonix error code to a description string.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param iErrorCode \b In: FxAnalysisFonix error code to convert.
/// \param iMaxChars \b In: size of the buffer allocated for \a oErrorString.
/// \param oErrorString \b Out: buffer for the description string.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixGetErrorString( const FxAnalysisFonixError iErrorCode,
							   const int iMaxChars,
							   wchar_t* oErrorString );

/// \brief Register a callback to receive updates on the percentage of analysis
/// completed.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param iCallbackFunction \b In: The address of callback function, or 
/// \c NULL to not receive any analysis progress updates.
/// \note The callback function signature must be 
/// \code void callbackFuntion(double percentComplete) \endcode
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixRegisterAnalysisCallback( void(*iCallbackFunction)(double) );
//@}

//-----------------------------------------------------------------------------
// Analysis Functions.
//-----------------------------------------------------------------------------
///\name Analysis Functions.
/// Functions to analyze audio data.
//@{
/// \brief Retrieves the minimum phoneme length (in seconds).
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixGetMinimumPhonemeLength( float& oMinPhonemeLength );

/// \brief Retrieves the minimum audio length (in seconds).
/// \return \c FX_ANSLYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixGetAudioMinimum( float& oAudioMin );

/// \brief Retrieves the maximum audio length (in seconds).
/// \return \c FX_ANSLYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixGetAudioMaximum( float& oAudioMax );

/// \brief Sets the minimum audio length (in seconds).
/// \note This function will clamp values less than zero to zero seconds.
/// \return \c FX_ANSLYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixSetAudioMinimum( float iAudioMin );

/// \brief Sets the maximum audio length (in seconds).
/// \note This function will clamp values lower than the internal maximum to 
/// the internal maximum.
/// \return \c FX_ANSLYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error
/// code if not.
FX_ANALYSIS_FONIX_API
FxAnalysisFonixError
FxAnalysisFonixSetAudioMaximum( float iAudioMax );

/// \brief Analyzes the specified audio data.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param iBitsPerSample \b In: bits per sample of the sound buffer.
/// \param iSampleRate \b In: sample rate of the sound buffer.
/// \param iChannelCount \b In:	channel count of the sound buffer.
/// \param iSampleCount \b In: number of samples in the sound buffer.
/// \param iSampleBuffer \b In:	pointer to the sound buffer.
/// \param iSoundTextSize \b In: size of iSoundText in bytes.
/// \param iSoundText \b In: text spoken in sound file (can be NULL to use 
/// no text analysis).
/// \param iLanguage \b In: language code for analysis (as returned from
/// FxAnalysisFonixGetSupportedLanguages()).
/// \param iDetectSpeech \b In: Whether to use Fonix's speech detection.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError
FxAnalysisFonixAnalyze( int iBitsPerSample,
						int iSampleRate,
						int iChannelCount,
						int iSampleCount,
						const char* iSampleBuffer,
						int iSoundTextSize,
						const wchar_t* iSoundText,
						int iLanguageCode,
						bool iDetectSpeech );
//@}

//-----------------------------------------------------------------------------
// Phoneme Functions.
//-----------------------------------------------------------------------------
///\name Phoneme Functions.
/// Functions to retrieve information related to the phonemes as segmented by 
/// FxAnalysisFonix.
//@{

/// \brief Gets the number of phonemes.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param oResult \b Out: number of phonemes.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixGetNumPhonemes( int& oResult ); 

/// \brief Gets phoneme information (enumeration, start time, end time) for the 
/// phoneme at index \a iPhonemeNum.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param iPhonemeNum \b In: phoneme index.
/// \param oEnum \b Out: enumeration of the specified phoneme.  This corresponds
/// to an FxPhoneme enumeration value.
/// \param oStartTime \b Out: start time of the phoneme in seconds.
/// \param oEndTime \b Out: end time of the phoneme in seconds.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixGetPhonemeInfo( const int iPhonemeNum,
 						       int& oEnum,
						       double& oStartTime,
						       double& oEndTime );
//@}

//-----------------------------------------------------------------------------
// Word Functions.
//-----------------------------------------------------------------------------
///\name Word Functions.
/// Functions to retrieve information related to the words as segmented by 
/// FxAnalysisFonix.
/// \note These functions are only valid if the analysis was done using text.
//@{

/// \brief Gets the number of words.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param oResult \b Out: number of words.
/// \note Only yields data for text-based analysis.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixGetNumWords( int& oResult );

/// \brief Gets the text of the specified word.
/// \return \c FX_ANALYSIS_FONIX_NOERROR if successful, FxAnalysisFonix error 
/// code if not.
/// \param iWordNum \b In: word index.
/// \param iMaxChars \b In: size of word buffer.
/// \param oWord \b Out: word buffer.
/// \param oStartTime \b Out: word start time in seconds.
/// \param oEndTime \b Out: word end time in seconds.
FX_ANALYSIS_FONIX_API 
FxAnalysisFonixError 
FxAnalysisFonixGetWordInfo( const int iWordNum,
						    const int iMaxChars,
							wchar_t* oWord,
							double& oStartTime,
							double& oEndTime );  
//@}

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
