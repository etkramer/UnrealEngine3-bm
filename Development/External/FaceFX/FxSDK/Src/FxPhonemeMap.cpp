//------------------------------------------------------------------------------
// A map from phonemes to tracks.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxPhonemeMap.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxPhonemeMapVersion 1
FX_IMPLEMENT_CLASS(FxPhonemeMap, kCurrentFxPhonemeMapVersion, FxObject)

FxPhonemeMap::FxPhonemeMap()
{
}

FxPhonemeMap::~FxPhonemeMap()
{
}

FxBool FxPhonemeMap::operator==( const FxPhonemeMap& other ) const
{
	// Early out if we have mismatched array lengths.
	if( _targetNames.Length() != other._targetNames.Length() ||
		_mapping.Length() != other._mapping.Length() )
	{
		return FxFalse;
	}

	FxSize numTargetNames = GetNumTargets();
	for( FxSize i = 0; i < numTargetNames; ++i )
	{
		if( GetTargetName(i) != other.GetTargetName(i) )
		{
			return FxFalse;
		}
	}

	FxSize numMappingEntries = GetNumMappingEntries();
	for( FxSize i = 0; i < numMappingEntries; ++i )
	{
		if( GetMappingEntry(i) != other.GetMappingEntry(i) )
		{
			return FxFalse;
		}
	}

	return FxTrue;
}

// Returns the phoneme mapping information for a phoneme/nameIndex pair.
FxReal FxPhonemeMap::GetMappingAmount( const FxPhoneme& phon, 
									   const FxName& name ) const
{
	FxSize numMappingEntries = _mapping.Length();
	for( FxSize i = 0; i < numMappingEntries; ++i )
	{
		if( _mapping[i].name == name && _mapping[i].phoneme == phon )
		{
			return _mapping[i].amount;
		}
	}
	return 0.0f;
}

FxReal FxPhonemeMap::GetMappingAmount( const FxPhoneme& phon, FxSize targetIndex ) const
{
	return GetMappingAmount(phon, GetTargetName(targetIndex));
}

// Sets the phoneme mapping information for a phoneme/nameIndex pair.
void FxPhonemeMap::SetMappingAmount( const FxPhoneme& phon, 
									 const FxName& name, FxReal amount )
{
	FxSize numMappingEntries = _mapping.Length();
	for( FxSize i = 0; i < numMappingEntries; ++i )
	{
		if( _mapping[i].name == name && _mapping[i].phoneme == phon )
		{
			if( !FxRealEquality(amount, 0.0f) )
			{
				// If we're not mapping to zero, update the entry.
				_mapping[i].amount = amount;
			}
			else
			{
				// If we're mapping an existing entry back to zero, remove it 
				// from the mapping to save memory.
				_mapping.Remove(i);
			}
			return;
		}
	}

	// If we're mapping a phoneme to a new target name, add it to the targets.
	FxBool newName = FxTrue;
	FxSize numTargetNames = _targetNames.Length();
	for( FxSize i = 0; i < numTargetNames; ++i )
	{
		newName &= _targetNames[i] != name;
	}
	if( newName )
	{
		_targetNames.PushBack(name);
	}

	// We don't need to add a mapping entry when the amount is 0 since any 
	// missing entry is assumed to be 0.
	if( !FxRealEquality(amount, 0.0f) )
	{
		FxPhonToNameMap newMappingInfo;
		newMappingInfo.phoneme = phon;
		newMappingInfo.name    = name;
		newMappingInfo.amount  = amount;
		_mapping.PushBack(newMappingInfo);
	}
}

void FxPhonemeMap::SetMappingAmount( const FxPhoneme& phon, const FxChar* name, FxReal amount )
{
	// Note the explicit FxName construction here so that the compiler does
	// not get confused now that FxNames are constructed from const FxChar*.
	SetMappingAmount(phon, FxName(name), amount);
}

void FxPhonemeMap::SetMappingAmount( const FxPhoneme& phon, FxSize targetIndex, FxReal amount )
{
	if( targetIndex < _targetNames.Length() )
	{
		SetMappingAmount(phon, GetTargetName(targetIndex), amount);
	}
}

// Removes all mapping entries associated with the given phoneme.
void FxPhonemeMap::RemovePhonemeMappingEntries( const FxPhoneme& phon )
{
	for( FxSize i = 0; i < _mapping.Length(); ++i )
	{
		if( _mapping[i].phoneme == phon )
		{
			_mapping.Remove(i--);
		}
	}
}

// Gets the number of non-zero mapping entries
FxSize FxPhonemeMap::GetNumMappingEntries( void ) const
{
	return _mapping.Length();
}

// Gets a requested mapping entry
const FxPhonToNameMap& FxPhonemeMap::GetMappingEntry( FxSize index ) const
{
	return _mapping[index];
}

// Adds a target to the mapping.
void FxPhonemeMap::AddTarget( const FxName& targetName )
{
	_targetNames.PushBack(targetName);
}

// Removes a target from the mapping.
void FxPhonemeMap::RemoveTarget( const FxName& targetName )
{
	for( FxSize i = 0; i < _mapping.Length(); ++i )
	{
		if( _mapping[i].name == targetName )
		{
			_mapping.Remove(i--);
		}
	}
	FxSize targetIndex = GetTargetIndex(targetName);
	if( targetIndex != FxInvalidIndex )
	{
		_targetNames.Remove(targetIndex);
	}
}

// Returns the number of targets.
FxSize FxPhonemeMap::GetNumTargets( void ) const
{
	return _targetNames.Length();
}

// Returns the nameIndex of the nth target.
const FxName& FxPhonemeMap::GetTargetName( FxSize n ) const
{
	return _targetNames[n];
}

// Returns the index of the target.
FxSize FxPhonemeMap::GetTargetIndex( const FxName& nameIndex ) const
{
	FxSize numTargetNames = _targetNames.Length();
	for( FxSize i = 0; i < numTargetNames; ++i )
	{
		if( _targetNames[i] == nameIndex ) return i;
	}
	return FxInvalidIndex;
}

// Clears the phoneme mapping.
void FxPhonemeMap::Clear( void )
{
	_targetNames.Clear();
	_mapping.Clear();
}

// Generates the "default" mapping.
void FxPhonemeMap::MakeDefaultMapping( void )
{
	Clear();
	SetMappingAmount(PHON_P,    "open",		0.00f);
	SetMappingAmount(PHON_B,    "open",		0.00f);
	SetMappingAmount(PHON_T,    "open",		0.40f);
	SetMappingAmount(PHON_D,    "open",		0.40f);
	SetMappingAmount(PHON_K,    "open",		0.25f);
	SetMappingAmount(PHON_G,    "open",		0.25f);
	SetMappingAmount(PHON_M,    "open",		0.00f);
	SetMappingAmount(PHON_N,    "open",		0.40f);
	SetMappingAmount(PHON_NG,   "open",		0.40f);
	SetMappingAmount(PHON_RA,   "open",		0.40f);
	SetMappingAmount(PHON_RU,   "open",		0.25f);
	SetMappingAmount(PHON_FLAP, "open",		0.30f);
	SetMappingAmount(PHON_PH,   "open",		0.10f);
	SetMappingAmount(PHON_F,	"open",		0.00f);
	SetMappingAmount(PHON_V,	"open",		0.00f);
	SetMappingAmount(PHON_TH,	"open",		0.45f);
	SetMappingAmount(PHON_DH,	"open",		0.45f);
	SetMappingAmount(PHON_S,	"open",		0.15f);
	SetMappingAmount(PHON_Z,	"open",		0.15f);
	SetMappingAmount(PHON_SH,	"open",		0.00f);
	SetMappingAmount(PHON_ZH,	"open",		0.00f);
	SetMappingAmount(PHON_CX,	"open",		0.25f);
	SetMappingAmount(PHON_X,	"open",		0.25f);
	SetMappingAmount(PHON_GH,	"open",		0.25f);
	SetMappingAmount(PHON_HH,	"open",		0.30f);
	SetMappingAmount(PHON_R,	"open",		0.10f);
	SetMappingAmount(PHON_Y,	"open",		0.00f);
	SetMappingAmount(PHON_L,	"open",		0.40f);
	SetMappingAmount(PHON_W,	"open",		0.00f);
	SetMappingAmount(PHON_H,	"open",		0.20f);
	SetMappingAmount(PHON_TS,	"open",		0.40f);
	SetMappingAmount(PHON_CH,	"open",		0.00f);
	SetMappingAmount(PHON_JH,	"open",		0.00f);
	SetMappingAmount(PHON_IY,	"open",		0.20f);
	SetMappingAmount(PHON_E,	"open",		0.35f);
	SetMappingAmount(PHON_EN,	"open",		0.35f);
	SetMappingAmount(PHON_EH,	"open",		0.50f);
	SetMappingAmount(PHON_A,	"open",		0.55f);
	SetMappingAmount(PHON_AA,	"open",		0.55f);
	SetMappingAmount(PHON_AAN,	"open",		0.55f);
	SetMappingAmount(PHON_AO,	"open",		0.40f);
	SetMappingAmount(PHON_AON,	"open",		0.40f);
	SetMappingAmount(PHON_O,	"open",		0.40f);
	SetMappingAmount(PHON_ON,	"open",		0.40f);
	SetMappingAmount(PHON_UW,	"open",		0.40f);
	SetMappingAmount(PHON_UY,	"open",		0.00f);
	SetMappingAmount(PHON_EU,	"open",		0.40f);
	SetMappingAmount(PHON_OE,	"open",		0.40f);
	SetMappingAmount(PHON_OEN,	"open",		0.40f);
	SetMappingAmount(PHON_AH,	"open",		0.50f);
	SetMappingAmount(PHON_IH,	"open",		0.50f);
	SetMappingAmount(PHON_UU,	"open",		0.40f);
	SetMappingAmount(PHON_UH,	"open",		0.40f);
	SetMappingAmount(PHON_AX,	"open",		0.50f);
	SetMappingAmount(PHON_UX,	"open",		0.50f);
	SetMappingAmount(PHON_AE,	"open",		0.50f);
	SetMappingAmount(PHON_ER,	"open",		0.40f);
	SetMappingAmount(PHON_AXR,	"open",		0.40f);
	SetMappingAmount(PHON_EXR,	"open",		0.40f);
	SetMappingAmount(PHON_EY,	"open",		0.50f);
	SetMappingAmount(PHON_AW,	"open",		0.50f);
	SetMappingAmount(PHON_AY,	"open",		0.50f);
	SetMappingAmount(PHON_OY,	"open",		0.40f);
	SetMappingAmount(PHON_OW,	"open",		0.40f);

	SetMappingAmount(PHON_R,	"W",		0.70f);
	SetMappingAmount(PHON_Y,	"W",		0.50f);
	SetMappingAmount(PHON_W,	"W",		0.85f);
	SetMappingAmount(PHON_AO,	"W",		0.55f);
	SetMappingAmount(PHON_AON,	"W",		0.55f);
	SetMappingAmount(PHON_O,	"W",		0.55f);
	SetMappingAmount(PHON_ON,	"W",		0.55f);
	SetMappingAmount(PHON_UW,	"W",		0.55f);
	SetMappingAmount(PHON_UY,	"W",		0.85f);
	SetMappingAmount(PHON_EU,	"W",		0.55f);
	SetMappingAmount(PHON_OE,	"W",		0.55f);
	SetMappingAmount(PHON_OEN,	"W",		0.55f);
	SetMappingAmount(PHON_UU,	"W",		0.55f);
	SetMappingAmount(PHON_UH,	"W",		0.55f);
	SetMappingAmount(PHON_OY,	"W",		0.55f);
	SetMappingAmount(PHON_OW,	"W",		0.55f);

	SetMappingAmount(PHON_SH,	"ShCh",		0.85f);
	SetMappingAmount(PHON_ZH,	"ShCh",		0.85f);
	SetMappingAmount(PHON_Y,	"ShCh",		0.30f);
	SetMappingAmount(PHON_CH,	"ShCh",		0.85f);
	SetMappingAmount(PHON_JH,	"ShCh",		0.85f);
	SetMappingAmount(PHON_ER,	"ShCh",		0.50f);
	SetMappingAmount(PHON_AXR,	"ShCh",		0.50f);
	SetMappingAmount(PHON_EXR,	"ShCh",		0.50f);

	SetMappingAmount(PHON_P,    "PBM",		0.90f);
	SetMappingAmount(PHON_B,    "PBM",		0.90f);
	SetMappingAmount(PHON_M,    "PBM",		0.90f);

	SetMappingAmount(PHON_PH,   "FV",		0.40f);
	SetMappingAmount(PHON_F,	"FV",		0.75f);
	SetMappingAmount(PHON_V,	"FV",		0.75f);

	SetMappingAmount(PHON_S,	"wide",		0.50f);
	SetMappingAmount(PHON_Z,	"wide",		0.50f);
	SetMappingAmount(PHON_IY,	"wide",		0.80f);
	SetMappingAmount(PHON_E,	"wide",		0.25f);
	SetMappingAmount(PHON_EN,	"wide",		0.25f);
	SetMappingAmount(PHON_EH,	"wide",		0.60f);
	SetMappingAmount(PHON_AH,	"wide",		0.60f);
	SetMappingAmount(PHON_IH,	"wide",		0.60f);
	SetMappingAmount(PHON_AX,	"wide",		0.60f);
	SetMappingAmount(PHON_UX,	"wide",		0.60f);	
	SetMappingAmount(PHON_AE,	"wide",		0.60f);
	SetMappingAmount(PHON_EY,	"wide",		0.60f);
	SetMappingAmount(PHON_AW,	"wide",		0.60f);
	SetMappingAmount(PHON_AY,	"wide",		0.60f);

	SetMappingAmount(PHON_K,    "tBack",	0.80f);
	SetMappingAmount(PHON_G,    "tBack",	0.80f);
	SetMappingAmount(PHON_RU,   "tBack",	0.80f);
	SetMappingAmount(PHON_CX,	"tBack",	0.80f);
	SetMappingAmount(PHON_X,	"tBack",	0.80f);
	SetMappingAmount(PHON_GH,	"tBack",	0.80f);
	SetMappingAmount(PHON_EH,	"tBack",	0.40f);
	SetMappingAmount(PHON_AH,	"tBack",	0.40f);
	SetMappingAmount(PHON_IH,	"tBack",	0.40f);
	SetMappingAmount(PHON_AX,	"tBack",	0.40f);
	SetMappingAmount(PHON_UX,	"tBack",	0.40f);	
	SetMappingAmount(PHON_AE,	"tBack",	0.40f);
	SetMappingAmount(PHON_EY,	"tBack",	0.40f);
	SetMappingAmount(PHON_AW,	"tBack",	0.40f);
	SetMappingAmount(PHON_AY,	"tBack",	0.40f);

	SetMappingAmount(PHON_T,    "tRoof",	0.80f);
	SetMappingAmount(PHON_D,    "tRoof",	0.80f);
	SetMappingAmount(PHON_N,    "tRoof",	0.80f);
	SetMappingAmount(PHON_NG,   "tRoof",	0.80f);
	SetMappingAmount(PHON_RA,   "tRoof",	0.50f);
	SetMappingAmount(PHON_FLAP, "tRoof",	0.60f);
	SetMappingAmount(PHON_S,	"tRoof",	0.40f);
	SetMappingAmount(PHON_Z,	"tRoof",	0.40f);
	SetMappingAmount(PHON_SH,	"tRoof",	0.40f);
	SetMappingAmount(PHON_ZH,	"tRoof",	0.40f);
	SetMappingAmount(PHON_Y,	"tRoof",	0.40f);
	SetMappingAmount(PHON_L,	"tRoof",	0.80f);
	SetMappingAmount(PHON_TS,	"tRoof",	0.80f);
	SetMappingAmount(PHON_CH,	"tRoof",	0.40f);
	SetMappingAmount(PHON_JH,	"tRoof",	0.40f);
	SetMappingAmount(PHON_IY,	"tRoof",	0.20f);
	SetMappingAmount(PHON_E,	"tRoof",	0.20f);
	SetMappingAmount(PHON_EN,	"tRoof",	0.20f);
	SetMappingAmount(PHON_ER,	"tRoof",	0.50f);
	SetMappingAmount(PHON_AXR,	"tRoof",	0.50f);
	SetMappingAmount(PHON_EXR,	"tRoof",	0.50f);

	SetMappingAmount(PHON_TH,	"tTeeth",	0.90f);
	SetMappingAmount(PHON_DH,	"tTeeth",	0.90f);
}

FxArchive& operator<<( FxArchive& arc, FxPhonToNameMap& obj )
{
	FxInt32 tempPhon = static_cast<FxInt32>(obj.phoneme);
	arc << tempPhon << obj.name << obj.amount;
	obj.phoneme = static_cast<FxPhoneme>(tempPhon);
	return arc;
}

struct FxPhonLookup
{
	FxPhoneme newPhon;
	FxPhoneme oldPhon;
};

void FxPhonemeMap::Serialize( FxArchive& arc )
{
	Super::Serialize( arc );

	FxUInt16 version = arc.SerializeClassVersion("FxPhonemeMap");
	
	arc << _mapping << _targetNames;

	// If we're loading the first version of a phoneme map, we need to remap our phonemes.
	if( arc.IsLoading() && version == 0 )
	{
		// Remove the old short silence phoneme, as it's no longer used.
		RemovePhonemeMappingEntries(static_cast<FxPhoneme>(41));
		for( FxSize i = 0; i < _mapping.Length(); ++i )
		{
			_mapping[i].phoneme = FxPhonemeUtility::UpdatePhoneme(_mapping[i].phoneme);
		}

		// Build the lookup table.
		FxPhonLookup phonLookup[] = {{PHON_RA, PHON_R}, {PHON_RU, PHON_R}, 
									 {PHON_PH, PHON_F}, {PHON_CX, PHON_K}, 
									 {PHON_X, PHON_K},  {PHON_GH, PHON_G}, 
									 {PHON_H, PHON_HH}, {PHON_TS, PHON_T},
									 {PHON_E, PHON_AA}, {PHON_EN, PHON_AA}, 
									 {PHON_A, PHON_AA}, {PHON_AAN, PHON_AA},
									 {PHON_AON, PHON_AO}, {PHON_O, PHON_AO}, 
									 {PHON_ON, PHON_AO}, {PHON_UY, PHON_W},
									 {PHON_EU, PHON_AO}, {PHON_OE, PHON_AO}, 
									 {PHON_OEN, PHON_AO}, {PHON_UU, PHON_W}, 
									 {PHON_UX, PHON_AX}, {PHON_AXR, PHON_ER}, 
									 {PHON_EXR, PHON_ER}};
		FxSize numLookupEntries = sizeof(phonLookup)/sizeof(FxPhonLookup);

		// If we're loading an old version of the mapping, we want to copy the
		// values from the old phonemes to the new phonemes.
		FxArray<FxPhonToNameMap> tempMappingEntries(numLookupEntries);
		FxSize numMappingEntries = _mapping.Length();
		for( FxSize i = 0; i < numLookupEntries; ++i )
		{
			// Find all entries in the mapping pertaining to the phoneme.
			for( FxSize j = 0; j < numMappingEntries; ++j )
			{
				if( _mapping[j].phoneme == phonLookup[i].oldPhon )
				{
					// Copy the mapping.
					FxPhonToNameMap entry(_mapping[j]);
					entry.phoneme = phonLookup[i].newPhon;
					tempMappingEntries.PushBack(entry);
				}
			}
		}
		for( FxSize i = 0; i < tempMappingEntries.Length(); ++i )
		{
			_mapping.PushBack(tempMappingEntries[i]);
		}
	}
}

} // namespace Face

} // namespace OC3Ent
