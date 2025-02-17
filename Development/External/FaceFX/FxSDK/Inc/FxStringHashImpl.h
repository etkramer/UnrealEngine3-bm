//------------------------------------------------------------------------------
// A structure to hash objects by a string key.
// Do not include this file.  Include FxStringHash.h instead.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

namespace OC3Ent
{

namespace Face
{

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::FxStringHash()
	: numInsertions(0)
	, numRemovals(0)
	, numCollisions(0)
	, numProbes(0)
{
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
		 FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::~FxStringHash()
{
	Clear();
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
template<FxSize OtherHashPrecisionBits,
		 template <typename> class OtherHashBinContainerType>
void 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::ConstructFrom( const FxStringHash<HashedObjectType, 
									OtherHashPrecisionBits,
									OtherHashBinContainerType>& other )
{
	typedef FxStringHash<HashedObjectType, 
						 OtherHashPrecisionBits,
						 OtherHashBinContainerType> OtherType;
	Clear();
	const FxSize numOtherBins = other.GetNumBins();
	for( FxSize binIndex = 0; binIndex < numOtherBins; ++binIndex )
	{
		const typename OtherType::HashBinType& theOtherBin  = other._getHashBin(binIndex);
		typename OtherType::HashBinType::ConstIterator curr = theOtherBin.Begin();
		typename OtherType::HashBinType::ConstIterator end  = theOtherBin.End();
		for( ; curr != end; ++curr )
		{
			const typename OtherType::FxHashBinEntry& entry = (*curr);
			Insert(entry.key.GetData(), entry.object);
		}
	}
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
void 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::Clear( void )
{
	const FxSize numBins = GetNumBins();
	for( FxSize binIndex = 0; binIndex < numBins; ++binIndex )
	{
		hashBins[binIndex].Clear();
	}
	numInsertions = 0;
	numRemovals   = 0;
	numCollisions = 0;
	numProbes     = 0;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
void 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::Trim( void )
{
	const FxSize numBins = GetNumBins();
	for( FxSize binIndex = 0; binIndex < numBins; ++binIndex )
	{
		if( hashBins[binIndex].IsEmpty() )
		{
			hashBins[binIndex].Clear();
		}
	}
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::Insert( const FxChar* key, const HashedObjectType& object )
{
	FxSize binIndex = _hash(key);
	HashBinType& theBin = hashBins[binIndex];
	if( !_findKeyInBin(key, theBin) )
	{
		if( !theBin.IsEmpty() )
		{
			numCollisions++;
		}
		numInsertions++;
		theBin.PushBack(FxHashBinEntry(key, object));
		return FxTrue;
	}
	return FxFalse;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::Remove( const FxChar* key )
{
	FxSize binIndex = _hash(key);
	return _removeFromBin(key, hashBins[binIndex]);
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::Remove( const FxChar* key, HashedObjectType& object )
{
	FxSize binIndex = _hash(key);
	return _removeFromBin(key, hashBins[binIndex], object);
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::Find( const FxChar* key ) const
{
	FxSize binIndex = _hash(key);
	return _findKeyInBin(key, hashBins[binIndex]);
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::Find( const FxChar* key, HashedObjectType& object ) const
{
	FxSize binIndex = _hash(key);
	return _findKeyInBin(key, hashBins[binIndex], object);
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxSize FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::GetNumBins( void ) const
{
	return FxHashSize(HashPrecisionBits);
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxSize 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::GetNumBinsInUse( void ) const
{
	FxSize numBinsInUse = 0;
	FxSize numBins = GetNumBins();
	for( FxSize binIndex = 0; binIndex < numBins; ++binIndex )
	{
		if( !hashBins[binIndex].IsEmpty() )
		{
			numBinsInUse++;
		}
	}
	return numBinsInUse;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxSize 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::GetMemUsedEstimate( void ) const
{
	FxSize numBins = GetNumBins();
	FxSize memUsedEstimate = numBins*sizeof(HashBinType);
	for( FxSize binIndex = 0; binIndex < numBins; ++binIndex )
	{
		const HashBinType& theBin = hashBins[binIndex];
		typename HashBinType::ConstIterator curr = theBin.Begin();
		typename HashBinType::ConstIterator end  = theBin.End();
		for( ; curr != end; ++curr )
		{
			const FxHashBinEntry& entry = (*curr);
			memUsedEstimate += sizeof(FxHashBinEntry) + 
				               (sizeof(FxChar) * (entry.key.Length()+1));
		}
	}
	return memUsedEstimate;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxSize FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::GetNumInsertions( void ) const
{
	return numInsertions;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxSize FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::GetNumRemovals( void ) const
{
	return numRemovals;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxSize FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::GetNumCollisions( void ) const
{
	return numCollisions;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxSize FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::GetNumProbes( void ) const
{
	return numProbes;
}

// This string hashing function was developed by Professor Daniel J. Bernstein 
// and was first published to comp.lang.c.  It is highly regarded as one of the 
// most efficient hash functions ever published.
template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxSize FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::_hash( const FxChar* key ) const
{
	// DJB's magic hash constant.
	FxSize binIndex = 5381;
	FxSize keyLength = FxStrlen(key);
	for( FxSize i = 0; i < keyLength; ++i )
	{
		// This is equivalent to binIndex * 33 + key[i].  33 is another one of
		// DJB's magic hash constants.
		binIndex = ((binIndex << 5) + binIndex) + key[i];
	}
	binIndex = (binIndex & FxHashMask(HashPrecisionBits));
#ifdef FX_XBOX_360
	// It is a safe assumption that the bin will be accessed pretty soon after
	// this, so go ahead and bring it into L1 and L2.
	__dcbt(binIndex*sizeof(HashBinType), hashBins);
#endif // FX_XBOX_360
	return binIndex;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::_removeFromBin( const FxChar* key, HashBinType& theBin )
{
	typename HashBinType::Iterator curr = theBin.Begin();
	typename HashBinType::Iterator end  = theBin.End();
	for( ; curr != end; ++curr )
	{
		FxHashBinEntry& entry = (*curr);
		if( entry.key == key )
		{
			numRemovals++;
			theBin.RemoveIterator(curr);
			return FxTrue;
		}
	}
	return FxFalse;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::_removeFromBin( const FxChar* key, HashBinType& theBin, 
				  HashedObjectType& object )
{
	typename HashBinType::Iterator curr = theBin.Begin();
	typename HashBinType::Iterator end  = theBin.End();
	for( ; curr != end; ++curr )
	{
		FxHashBinEntry& entry = (*curr);
		if( entry.key == key )
		{
			numRemovals++;
			object = entry.object;
			theBin.RemoveIterator(curr);
			return FxTrue;
		}
	}
	return FxFalse;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::_findKeyInBin( const FxChar* key, const HashBinType& theBin ) const
{
	typename HashBinType::ConstIterator curr = theBin.Begin();
	typename HashBinType::ConstIterator end  = theBin.End();
	for( ; curr != end; ++curr )
	{
		const FxHashBinEntry& entry = (*curr);
		if( entry.key == key )
		{
			numProbes++;
			return FxTrue;
		}
	}
	return FxFalse;
}

template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
		 template <typename> class HashBinContainerType>
FxBool FX_INLINE 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::_findKeyInBin( const FxChar* key, const HashBinType& theBin, 
				 HashedObjectType& object ) const
{
	typename HashBinType::ConstIterator curr = theBin.Begin();
	typename HashBinType::ConstIterator end  = theBin.End();
	for( ; curr != end; ++curr )
	{
		const FxHashBinEntry& entry = (*curr);
		if( entry.key == key )
		{
			numProbes++;
			object = entry.object;
			return FxTrue;
		}
	}
	return FxFalse;
}


template<typename HashedObjectType, 
		 FxSize HashPrecisionBits,
	     template <typename> class HashBinContainerType>
FX_INLINE const typename FxStringHash<HashedObjectType, 
									  HashPrecisionBits,
									  HashBinContainerType>::HashBinType& 
FxStringHash<HashedObjectType, HashPrecisionBits, HashBinContainerType>
::_getHashBin( FxSize index ) const
{
	return hashBins[index];
}

} // namespace Face

} // namespace OC3Ent
