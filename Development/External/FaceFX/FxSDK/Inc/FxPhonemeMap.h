//------------------------------------------------------------------------------
// A map from phonemes to tracks.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPhonemeMap_H__
#define FxPhonemeMap_H__

#include "FxPlatform.h"
#include "FxObject.h"
#include "FxPhonemeEnum.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

/// A single entry in the FxPhonemeMap.
class FxPhonToNameMap
{
public:
	/// Tests for equal mapping entries.
	FxBool operator==( const FxPhonToNameMap& other ) const;
	/// Tests for non-equal mapping entries.
	FxBool operator!=( const FxPhonToNameMap& other ) const;

	FxPhoneme phoneme;	///< The phoneme.
	FxName    name;		///< The track.
	FxReal    amount;	///< The mapping amount.
};

/// A map from phonemes to tracks.
class FxPhonemeMap : public FxObject
{
	// Declare the class.
	FX_DECLARE_CLASS(FxPhonemeMap, FxObject)
public:
	/// Constructor.
	FxPhonemeMap();
	/// Destructor.
	~FxPhonemeMap();

	/// Returns whether two phoneme mappings are identical.
	/// \param other The phoneme mapping to compare against.
	/// \return \p FxTrue if the mappings are identical, \p FxFalse otherwise.
	FxBool operator==( const FxPhonemeMap& other ) const;

	/// Returns the phoneme mapping information for a phoneme/name pair.
	/// \param phon The phoneme enumeration.
	/// \param name The name of the track.
	/// \return The mapping amount between \a phon and \a name.
	FxReal GetMappingAmount( const FxPhoneme& phon, const FxName& name ) const;
	/// Returns the phoneme mapping information for a phoneme/name pair.
	/// \param phon The phoneme enumeration.
	/// \param targetIndex The index of the track, from GetTargetIndex().
	/// \return The mapping amount between \a phon and \a name.
	FxReal GetMappingAmount( const FxPhoneme& phon, FxSize targetIndex ) const;

	/// Sets the phoneme mapping information for a phoneme/name pair.
	/// \param phon The phoneme enumeration.
	/// \param name The name of the track.
	/// \param amount The amount to map from \a phon to \a name.
	void SetMappingAmount( const FxPhoneme& phon, const FxName& name, FxReal amount );
	/// Sets the phoneme mapping information for a phoneme/name pair.
	/// \param phon The phoneme enumeration.
	/// \param name The name of the track.
	/// \param amount The amount to map from \a phon to \a name.
	void SetMappingAmount( const FxPhoneme& phon, const FxChar* name, FxReal amount );
	/// Sets the phoneme mapping information for a phoneme/name pair.
	/// \param phon The phoneme enumeration.
	/// \param targetIndex The index of the track, from GetTargetIndex().
	/// \param The amount to map from \a phon to \a targetIndex.
	void SetMappingAmount( const FxPhoneme& phon, FxSize targetIndex, FxReal amount );

	/// Removes all mapping entries associated with the given phoneme.
	/// \param phon The phoneme from which to clear mapping entries.
	void RemovePhonemeMappingEntries( const FxPhoneme& phon );

	/// Gets the number of non-zero mapping entries.
	FxSize GetNumMappingEntries( void ) const;
	/// Gets a requested mapping entry.
	/// \param The index of a mapping entry, from zero to GetNumMappingEntries().
	/// \return The mapping entry.
	const FxPhonToNameMap& GetMappingEntry( FxSize index ) const;

	/// Adds a target to the mapping.
	/// \param targetName The name of the target to add to the mapping.
	void AddTarget( const FxName& targetName );
	/// Removes a target from the mapping.
	/// \param targetName The name of the target to remove from the mapping.
	void RemoveTarget( const FxName& targetName );

	/// Returns the number of targets.
	FxSize GetNumTargets( void ) const;
	/// Returns the name of a target.
	/// \param targetIndex The index of the target.
	/// \return The name of the target at \a targetIndex.
	const FxName& GetTargetName( FxSize targetIndex ) const;
	/// Returns the index of the target.
	/// \param name The name of the target.
	/// \return The index of the 
	FxSize GetTargetIndex( const FxName& name ) const;

	/// Clears the phoneme mapping.
	void Clear( void );
	/// Generates the "default" mapping.
	void MakeDefaultMapping( void );

	/// Serialization.
	virtual void Serialize( FxArchive& arc );

protected:
	// The mapping information.
	FxArray<FxPhonToNameMap> _mapping;
	FxArray<FxName> _targetNames;
};

FX_INLINE FxBool FxPhonToNameMap::operator==(const FxPhonToNameMap& other) const
{
	return name == other.name && phoneme == other.phoneme && 
		   FxRealEquality(amount, other.amount);
}

FX_INLINE FxBool FxPhonToNameMap::operator!=(const FxPhonToNameMap& other) const
{
	return !operator==(other);
}

} // namespace Face

} // namespace OC3Ent

#endif
