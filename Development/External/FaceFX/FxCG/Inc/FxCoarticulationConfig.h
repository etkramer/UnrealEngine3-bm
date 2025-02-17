//------------------------------------------------------------------------------
// The parameters for the coarticulation.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCoarticulationConfig_H__
#define FxCoarticulationConfig_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxNamedObject.h"

namespace OC3Ent
{

namespace Face
{

struct FxPerPhoneCoarticulationInfo
{
	FxPerPhoneCoarticulationInfo( FxReal iDominance = 0.5f, FxReal iAnticipation = 0.03f)
		: dominance(iDominance)
		, anticipation(iAnticipation)
	{
	}

	FxReal dominance;
	FxReal anticipation;
};

class FxCoarticulationConfig : public FxNamedObject
{
	// Declare the class.
	FX_DECLARE_CLASS(FxCoarticulationConfig, FxNamedObject);

public:
	FxCoarticulationConfig();

	FxReal leadInTime;                // The time to lead into a phoneme following SIL.
	FxReal leadOutTime;               // The time to lead out of a phoneme following SIL.
	FxReal shortSilenceDuration;      // The length of a silence to consider "short".
	FxReal suppressionWindowMax;      // The phoneme length at which phoneme suppression starts even for the most 
	                                  // dominant phonemes.
	FxReal suppressionWindowMaxSlope; // A co-efficient multiplied by the phoneme dominance ratio to determine how much 
	                                  // the supressionWindowMax variable is increased for non-dominant phonemes.
	FxReal suppressionWindowMin;      // The phoneme length at which the most dominant phonemes will be fully suppressed.
	FxReal suppressionWindowMinSlope; // A co-efficient multiplied by the phoneme dominance ratio to determine how much 
	                                  // the supressionWindowMin variable is increased for non-dominant phonemes.
	FxBool splitDiphthongs;           // Whether or not to break diphthongs into their component vowels.
	FxReal diphthongBoundary;         // The smallest time at which diphthongs are not converted to component vowels.
	FxBool convertToFlaps;            // Whether or not to convert appropriate consonants to flaps.	
	FxArray<FxPerPhoneCoarticulationInfo> phoneInfo; // Per-phoneme information.

	FxString		description; // The description of the configuration.

	// Serialize the configuration
	void Serialize( FxArchive& arc );
};

/// Utility function to load a coarticulation config from a file on disc.
/// \param actor A reference to the coarticulation config to load into.
/// \param filename The path to the file on disc to load from.
/// \param bUseFastMethod If \p FxTrue FxArchiveStoreFileFast is used internally.
/// If \p FxFalse FxArchiveStoreFile is used.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxLoadCoarticulationConfigFromFile( 
	FxCoarticulationConfig& config, const FxChar* filename, const FxBool bUseFastMethod,
	void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );

/// Utility function to save a coarticulation config to a file on disc.
/// \param actor A reference to the actor to save from.
/// \param filename The path to the file on disc to save to.
/// \param byteOrder The byte order to save in.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxSaveCoarticulationConfigToFile( 
	FxCoarticulationConfig& config, const FxChar* filename, 
	FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian,
	void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );

/// Utility function to load a coarticulation config from a block of memory.
/// \param actor A reference to the coarticulation config to load into.
/// \param pMemory The array of bytes containing the data to load from.
/// \param numBytes The size, in bytes, of pMemory.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxLoadCoarticulationConfigFromMemory( 
	FxCoarticulationConfig& config, const FxByte* pMemory, const FxSize numBytes,
	void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );

/// Utility function to save a coarticulation config to a block of memory.
/// \param actor A reference to the coarticulation config to save from.
/// \param pMemory A reference to a pointer that will hold the array of bytes
/// containing the actor data.  This must be NULL when passed in and will be
/// allocated internally via FxAlloc().  The client is responsible for freeing 
/// this memory and it must be freed via a call to FxFree().
/// \param numBytes Upon success, this is the size, in bytes, of pMemory.
/// \param byteOrder The byte order to save in.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxSaveCoarticulationConfigToMemory( 
	FxCoarticulationConfig& config,	FxByte*& pMemory, FxSize& numBytes, 
	FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian,
	void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );


} // namespace Face

} // namespace OC3Ent

#endif
