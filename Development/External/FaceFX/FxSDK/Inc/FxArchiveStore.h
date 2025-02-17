//------------------------------------------------------------------------------
// This is the pure virtual base class for all types of archive stores.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxArchiveStore_H__
#define FxArchiveStore_H__

#include "FxPlatform.h"
#include "FxMemory.h"

namespace OC3Ent
{

namespace Face
{

/// The pure virtual base class for all types of archive stores
/// \ingroup object
class FxArchiveStore : public FxUseAllocator
{
public:
	friend class FxArchive;
	
	/// Returns FxTrue if the archive store is in a valid state.
	virtual FxBool IsValid( void ) const = 0;
	/// Opens the archive store for reading.
	virtual FxBool OpenRead( void ) = 0;
	/// Opens the archive store for writing.
	virtual FxBool OpenWrite( void ) = 0;
	/// Closes the archive store.
	virtual FxBool Close( void ) = 0;
	/// Reads size bytes from the archive store and puts them in data.
	virtual FxBool Read( FxByte* data, FxSize size ) = 0;
	/// Writes size bytes from data into the archive store.
	virtual FxBool Write( const FxByte* data, FxSize size ) = 0;
	/// Returns the current position in the store relative to the beginning.
	virtual FxInt32 Tell( void ) const = 0;
	/// Sets the current position in the store relative to the beginning.
	virtual void Seek( const FxInt32 pos ) = 0;
	/// Returns the total length of the store.
	virtual FxInt32 Length( void ) const = 0;

protected:
	/// Constructor.
	FxArchiveStore() {}
	/// Destructor.
	virtual ~FxArchiveStore() {}
	/// Destroys the archive store.  This must be overridden to call
	/// the appropriate destructor.
	virtual void Destroy( void ) = 0;
	/// Returns the size of the archive store class.  This must be 
	/// overridden to return static_cast<FxSize>(sizeof(NewArchiveStoreClass)).
	virtual FxSize GetClassSize( void ) = 0;

private:
    /// Disable copy construction.
	FxArchiveStore( const FxArchiveStore& );
	/// Disable assignment.
	FxArchiveStore& operator=( const FxArchiveStore& );
};

} // namespace Face

} // namespace OC3Ent

#endif
