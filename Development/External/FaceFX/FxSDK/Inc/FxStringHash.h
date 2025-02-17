//------------------------------------------------------------------------------
// A structure to hash objects by a string key.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxStringHash_H__
#define FxStringHash_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxArchive.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

// Some handy macros for the hashing functionality.
// Computes 2^x.
#define FxHashSize(x) ((FxSize)1<<(x))
// Computes 2^x - 1.
#define FxHashMask(x) (FxHashSize(x)-1)

/// The default value of HashPrecisionBits for FxStringHash.  This results in
/// 2^12 (4096) hash bins in the hash table.
static const FxSize FxDefaultHashPrecisionBits = 12;

/// A templated structure to hash objects by string keys.  Note that this 
/// structure simply stores an object in the hash based on the key provided, so
/// it is invalid for two unique objects to share the same key simultaneously.
/// In that case the insertion of the second object will fail because there 
/// is already an object with its key in the hash.  Make note of this when using
/// FxStringHash as it is assumed that each key / object pair in the hash is 
/// unique and does not compare the actual objects when searching (only the keys 
/// themselves are compared).
///
/// HashedObjectType is the type of object to be hashed by string.
///
/// HashBinContainerType is the type of container used in each hash bin.  This
/// defaults to FxArray, but can be set to any FaceFX container class type
/// (e.g. FxList is also a valid HashBinContainerType).
///
/// HashPrecisionBits is the number of bits used in the hashing functionality.
/// This defaults to FxDefaultHashPrecisionBits (12) resulting in 2^12 (4096)
/// hash bins and the result of the hashing function is masked
/// to fit in range [0, 2^HashPrecisionBits - 1].  The hashing function is 
/// optimized to result in zero collisions in 2^32.  Since a hash table with 
/// 2^32 elements is completely out of scope, we need to mask the 32-bit result 
/// to fit in in the hash table.  As a result, the hashing function will produce 
/// more collisions than it should theoretically.
///
/// \ingroup support
template<typename HashedObjectType, 
		 FxSize HashPrecisionBits = FxDefaultHashPrecisionBits,
		 template <typename> class HashBinContainerType = FxArray> 
class FxStringHash
{
private:
	// Forward declare FxHashBinEntry.
	class FxHashBinEntry;
public:
	/// The type of container used in each hash bin.
	typedef HashBinContainerType<FxHashBinEntry> HashBinType;

	/// Constructor.
	FxStringHash();
	/// Destructor.
	~FxStringHash();

	/// Constructs the FxStringHash from the other FxStringHash, even if the
	/// other's HashBinContainerType or HashPrecisionBits is different from
	/// the current setting.  This is mostly useful for backwards compatible
	/// serialization.  Note that the HashedObjectType must be the same or this
	/// template function will not compile.
	template<FxSize OtherHashPrecisionBits,
			 template <typename> class OtherHashBinContainerType>
		void ConstructFrom( const FxStringHash<HashedObjectType, 
										OtherHashPrecisionBits,
										OtherHashBinContainerType>& other );

	/// Clears all items from the hash table.  This also resets the various
	/// statistics gathering member variables.
	void Clear( void );
	/// Trims all empty hash bins in the hash table.  If a hash bin is empty
	/// but has memory allocated, this will reclaim that memory.
	void Trim( void );

	/// Inserts \a object using \a key into the hash table.
	/// \returns FxTrue if \a object was inserted or FxFalse if not.  FxFalse
	/// can be returned if the \a key / \a object pair is already in the hash.
	FxBool Insert( const FxChar* key, const HashedObjectType& object );
	/// Removes the object using \a key from the hash table.
	/// \returns FxTrue if the object was removed or FxFalse if it was not.
	/// FxFalse can be returned if \a key is not in the hash.
	FxBool Remove( const FxChar* key );
	/// Removes the object using \a key from the hash table and sets \a object
	/// equal to the object in the hash.
	/// \returns FxTrue if the object was removed and \a object is valid or 
	/// FxFalse if it was not.
	/// FxFalse can be returned if \a key is not in the hash.
	FxBool Remove( const FxChar* key, HashedObjectType& object );
	/// Finds the object using \a key in the hash table.
	/// \returns FxTrue if an object using \a key was found or FxFalse if it 
	/// was not.  FxFalse can be returned if \a key is not in the hash.
	FxBool Find( const FxChar* key ) const;
	/// Finds the object using \a key in the hash table and sets \a object equal
	/// to the object in the hash.
	/// \returns FxTrue if an object using \a key was found and \a object is 
	/// valid or FxFalse if it was not.  FxFalse can be returned if \a key is 
	/// not in the hash.
	FxBool Find( const FxChar* key, HashedObjectType& object ) const;

	/// Returns the number of bins contained in the hash table.
	FxSize GetNumBins( void ) const;
	/// Returns the number of bins currently in use (non-empty) in the hash 
	/// table.
	FxSize GetNumBinsInUse( void ) const;
	/// Returns an estimate of the memory used by the hash table.
	FxSize GetMemUsedEstimate( void ) const;
	/// Returns the total number of insertion operations performed.
	FxSize GetNumInsertions( void ) const;
	/// Returns the total number of removal operations performed.
	FxSize GetNumRemovals( void ) const;
	/// Returns the total number of collisions that have occurred in the hash 
	/// table.  A collision is defined as inserting a key / object pair into a 
	/// hash bin that already contains entries.
	FxSize GetNumCollisions( void ) const;
	/// Returns the total number of probes that have occurred in the hash table.
	/// A probe is defined as searching for a key / object pair that is already
	/// contained in the hash.
	FxSize GetNumProbes( void ) const;

private:
	/// An entry in a hash bin.
	class FxHashBinEntry
	{
	public:
		/// Constructor.
		FxHashBinEntry() {}
		/// Constructor.
		FxHashBinEntry( const FxString& iKey, const HashedObjectType& iObject )
			: key(iKey)
			, object(iObject) {}
			/// Copy constructor.
			FxHashBinEntry( const FxHashBinEntry& other )
				: key(other.key)
				, object(other.object) {}
				/// Assignment operator.
				FxHashBinEntry& operator=( const FxHashBinEntry& other )
				{
					if( this == &other ) return *this;
					key    = other.key;
					object = other.object;
					return *this;
				}

				// Hash bin entries are serialized in place in the 
				// string hash's serialization function below.

				/// The entry's key.
				FxString key;
				/// The entry's object.
				HashedObjectType object;
	};

public:
	/// Serializes the FxStringHash to an archive.
	FxArchive& Serialize( FxArchive& arc )
	{
		if( arc.IsLoading() )
		{
			Clear();
		}

		FxSize numBinsInUse = 0;
		if( arc.IsSaving() )
		{
			// Get the number of in-use bins if saving.
			numBinsInUse = GetNumBinsInUse();
		}
		arc << numBinsInUse;

		// Serialize a sparse representation of the hash but assert if any bin 
		// is loaded that has an index greater than or equal to the number of 
		// bins that this FxStringHash contains.  This can happen if e.g. an 
		// FxStringHash was saved in an archive with HashPrecisionBits set to 
		// 8 (2^8 = 256) bins, the code was updated to only have 
		// HashPrecisionBits set to 6 (2^6 = 64) bins, and then the archive 
		// containing 256 bins is loaded.  If backwards compatible serialization 
		// is needed after changing either the HashPrecisionBits or 
		// HashBinContainerType setting in code, special care must be taken to 
		// load old archives at a higher level than this.  For example, 
		// increment a version number, serialize with the old HashPrecisionBits 
		// setting, then rehash with the new HashPrecisionBits or 
		// HashBinContainerType setting:
		//
		//     FxStringHash<FxSize, FxArray, 6> newHashType;
		//     if( versionNumberFromArchive == oldHashVersionNumber )
		//     {
		//         FxStringHash<FxSize, FxList, 8> oldHashType;
		//         arc << oldHashType;
		//         newHashType.ConstructFrom(oldHashType);
		//     }
		FxSize numTotalBins = GetNumBins();
		if( arc.IsSaving() )
		{
			// Save out a sparse representation of the hash.
			for( FxSize binIndex = 0; binIndex < numTotalBins; ++binIndex )
			{
				// Save out any non-empty bins.
				if( !hashBins[binIndex].IsEmpty() )
				{
					arc << binIndex;

					// Serialize the bin contents directly.
					HashBinType& theBin = hashBins[binIndex];
					FxSize len = theBin.Length();
					arc << len;
					typename HashBinType::Iterator curr = theBin.Begin();
					typename HashBinType::Iterator end  = theBin.End();
					for( ; curr != end; ++curr )
					{
						FxHashBinEntry& entry = (*curr);
						arc << entry.key << entry.object;
					}
				}
			}
		}
		else if( arc.IsLoading() )
		{
			for( FxSize binIndex = 0; binIndex < numBinsInUse; ++binIndex )
			{
				FxSize serializedBinIndex = 0;
				arc << serializedBinIndex;
				// Make sure serializedBinIndex is in range and assert if not.
				// See the comments above for how that could happen.
				FxAssert(serializedBinIndex < numTotalBins);
				// Serialize the array contents directly.
				FxSize serializedBinLen = 0;
				arc << serializedBinLen;
				for( FxSize i = 0; i < serializedBinLen; ++i )
				{
					FxHashBinEntry serializedEntry;
					arc << serializedEntry.key << serializedEntry.object;
					hashBins[serializedBinIndex].PushBack(serializedEntry);
				}
			}
		}

		return arc;
	}

private:

	/// Hashes the specified key and automatically masks the result to 
	/// FxHashMask(HashPrecisionBits) bits ensuring that the result will be 
	/// between zero and FxHashSize(HashPrecisionBits).
	FxSize _hash( const FxChar* key ) const;
	/// Removes the object using \a key from \a theBin.
	/// \returns FxTrue if the object was removed or FxFalse if it was not.
	/// FxFalse can be returned if \a key is not in \a theBin.
	FxBool _removeFromBin( const FxChar* key, HashBinType& theBin );
	/// Removes the object using \a key from \a theBin and sets \a object
	/// equal to the object in \a theBin.
	/// \returns FxTrue if the object was removed and \a object is valid or 
	/// FxFalse if it was not.
	/// FxFalse can be returned if \a key is not in \a theBin.
	FxBool _removeFromBin( const FxChar* key, HashBinType& theBin, 
		                   HashedObjectType& object );
	/// Finds the object using \a key in the \a theBin.
	/// \returns FxTrue if an object using \a key was found or FxFalse if it 
	/// was not.  FxFalse can be returned if \a key is not in \a theBin.
	FxBool _findKeyInBin( const FxChar* key, const HashBinType& theBin ) const;
	/// Finds the object using \a key in \a theBin and sets \a object equal
	/// to the object in \a theBin.
	/// \returns FxTrue if an object using \a key was found and \a object is 
	/// valid or FxFalse if it was not.  FxFalse can be returned if \a key is 
	/// not in \a theBin.
	FxBool _findKeyInBin( const FxChar* key, const HashBinType& theBin, 
		                  HashedObjectType& object ) const;
	/// Returns a const reference to the HashBinType at index.
	const HashBinType& _getHashBin( FxSize index ) const;

	/// The total number of insertion operations performed.
	mutable FxSize numInsertions;
	/// The total number of removal operations performed.
	mutable FxSize numRemovals;
	/// The total number of collisions that have occurred in the hash table.  A 
	/// collision is defined as inserting a key / object pair into a hash bin 
	/// that already contains entries.
	mutable FxSize numCollisions;
	/// The total number of probes that have occurred in the hash table.  A 
	/// probe is defined as searching for a key / object pair that is already 
	/// contained in the hash.
	mutable FxSize numProbes;
	/// The hash table.  This is 2^HashPrecisionBits in length.
	HashBinType hashBins[FxHashSize(HashPrecisionBits)];
};

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits, 
		 template <typename> class HashBinContainerType>
FX_INLINE FxArchive& operator<<(FxArchive& arc, 
								class FxStringHash<HashedObjectType, 
								    			   HashPrecisionBits, 
												   HashBinContainerType>& hash)
{
	return hash.Serialize(arc);
}

} // namespace Face

} // namespace OC3Ent

#include "FxStringHashImpl.h"

#endif
