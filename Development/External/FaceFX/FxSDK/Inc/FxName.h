//------------------------------------------------------------------------------
// This class is responsible for managing the names of all the named objects
// in the system.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxName_H__
#define FxName_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxStringHash.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare FxRefString.
class FxRefString;

/// An object name.  This class employs the FxRefString and a name table to 
/// ensure that each name exists only once in memory.  Not only does this result
/// in memory savings, but also provides a very fast name comparison by only
/// checking the pointers to the reference-counted strings.
/// \ingroup support
class FxName
{
public:
	/// Default constructor.
	FxName();
	/// Constructs a name.
	FxName( const FxChar* name );
	/// Copy constructs a name.
	FxName( const FxName& other );
	/// Assignment from const FxChar*.
	FxName& operator=( const FxChar* other );
    /// Assignment from FxName.
	FxName& operator=( const FxName& other );
    /// Destructor.
	~FxName();

	/// Starts up and initializes the name system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Startup( void );
	/// Shuts down the name system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Shutdown( void );

	/// Equality test with another name.
	FxBool operator==( const FxName& other ) const;
	/// Equality test with a string.
	FxBool operator==( const FxChar* other ) const;
	/// Inequality test with another name.
	FxBool operator!=( const FxName& other ) const;
	/// Inequality test with a string.
	FxBool operator!=( const FxChar* other ) const;
	
	/// Sets the name.
	void SetName( const FxChar* name );
	/// Gets the name as a string.
	const FxString& GetAsString( void ) const;
	/// Gets the name as a c-style string.
	const FxChar* GetAsCstr( void ) const;
	/// Replaces the underlying string, effectively renaming all objects which
	/// use the same name.  Note that this will perform sanity checks to make
	/// sure that a FaceFX "internal" name is not being renamed and as such
	/// should not be called in-game for performance reasons.  This is generally
	/// a FaceFX Studio-only operation.
	/// \note Internal use only.
	void Rename( const FxChar* name );

	/// Returns the number of names in the name table.
	static FxSize FX_CALL GetNumNames( void );
	/// Finds a name in the name table and returns its index.
	/// \param nameToFind the name to find.
	/// \return The index if found, otherwise FxInvalidIndex.
	static FxSize FX_CALL Find( const FxChar* nameToFind );
	/// Returns the number of bytes used for string storage in the name table.
	/// This is the number of characters in all strings in the name table 
	/// excluding class and container overhead.
	static FxSize FX_CALL GetNameTableSize( void );
	/// Returns a string representation of the name in the name table at index.
	static const FxString* FX_CALL GetNameAsString( FxSize index );
	/// Returns the number of references for a given name.
	/// \param index The index of the name in the name table.
	/// \return The number of references to the name, or FxInvalidIndex if the name
	/// table has not been created.
	static FxSize FX_CALL GetNumReferences( FxSize index );

	/// Returns basic statistics from the underlying name hash.
	/// \param numCollisions The total number of collisions that have occurred
	/// in the name hash.
	/// \param numNamesAdded The total number of names that have been added to
	/// the name hash.
	/// \param numProbes The total number of names that did not need to 
	/// be added to the name hash because they were already there.
	/// \param numHashBins The total number of hash bins in the name hash.
	/// \param numUsedHashBins The total number of hash bins that are currently
	/// in use in the name hash.
	/// \param memUsed The approximate memory usage (in bytes) of just the
	/// name hash structure.  This does not include the memory usage of the
	/// actual name table.  This total includes stack space as well as heap 
	/// space used.
	static void FX_CALL GetNameStats( FxSize& numCollisions, FxSize& numNamesAdded,
		                              FxSize& numProbes, FxSize& numHashBins, 
							          FxSize& numUsedHashBins, FxSize& memUsed );

	/// Returns the underlying reference counted string representing the name.
	/// \note Internal use only.
	FxRefString** GetRefString( void ) { return _ppRefString; }

	/// The "NULL" name.
	static FxName NullName;
	
	/// Serializes the name to an archive.
	friend FxArchive& operator<<( FxArchive& arc, FxName& name );

private:
	/// Adds a name to the table avoiding duplicates.
	void _addNameToTable( const FxChar* name );
	/// Safely removes a reference to a name.
	void _safeRemoveRef( void );

	/// The global name hash.  The name hash is simply an auxiliary structure that 
	/// sits conceptually in front of the name table itself providing name lookup
	/// in O(1) time.  It is very fast to determine if a name is already in the 
	/// name table or not.  The only time any memory is allocated is when the name
	/// is not already in the table and therefore needs to be added.
	static FxStringHash<FxRefString**> _hashTable;

	/// The reference counted string representing the name.
	mutable FxRefString** _ppRefString;
};

FxArchive& operator<<( FxArchive& arc, FxName& name );

FX_INLINE FxBool FxName::operator==( const FxName& other ) const
{
	if( _ppRefString )
	{
		FxAssert(_ppRefString != NULL && other._ppRefString != NULL);
		return *_ppRefString == *other._ppRefString;
	}
	else
	{
		_ppRefString = NullName._ppRefString;
		FxAssert(_ppRefString != NULL && other._ppRefString != NULL);
		return *_ppRefString == *other._ppRefString;
	}
}

FX_INLINE FxBool FxName::operator!=( const FxName& other ) const
{
	if( _ppRefString )
	{
		FxAssert(_ppRefString != NULL && other._ppRefString != NULL);
		return *_ppRefString != *other._ppRefString;
	}
	else
	{
		_ppRefString = NullName._ppRefString;
		FxAssert(_ppRefString != NULL && other._ppRefString != NULL);
		return *_ppRefString != *other._ppRefString;
	}
}

} // namespace Face

} // namespace OC3Ent

#endif
