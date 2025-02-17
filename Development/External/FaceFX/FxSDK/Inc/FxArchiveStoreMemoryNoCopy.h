//------------------------------------------------------------------------------
// A memory-based archive store that does not make an internal copy of the
// memory passed in.  This archive store can only be used for loading and 
// prevents saving.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxArchiveStoreMemoryNoCopy_H__
#define FxArchiveStoreMemoryNoCopy_H__

#include "FxPlatform.h"
#include "FxArchiveStore.h"

namespace OC3Ent
{

namespace Face
{

/// An archive store for working in memory without creating an internal copy
/// of the memory passed in.
/// Useful for:
/// \li Loading from FaceFX data embedded in other game assets.
/// \ingroup object
class FxArchiveStoreMemoryNoCopy : public FxArchiveStore
{
public:
	friend class FxArchiveStoreFileFast;

	/// Creates an FxArchiveStoreMemory object.
	/// \param memory A pointer to the chunk from which to load.
	/// \param size The length of the chunk pointed to by \a memory, in bytes.
	/// \return A pointer to a new archive store.
	/// \note Do not free \a memory until serialization is complete.
	static FxArchiveStoreMemoryNoCopy* Create( const FxByte* memory, FxSize size );

	/// Returns FxTrue if the archive store is in a valid state.
	virtual FxBool IsValid( void ) const;
	/// Opens the archive store for reading.
	virtual FxBool OpenRead( void );
	/// Opens the archive store for writing.
	virtual FxBool OpenWrite( void );
	/// Closes the archive store.
	virtual FxBool Close( void );
	/// Reads size bytes from the archive store and puts them in data.
	virtual FxBool Read( FxByte* data, FxSize size );
	/// Writes size bytes from data into the archive store.
	virtual FxBool Write( const FxByte* data, FxSize size );
	/// Returns the current position in the store relative to the beginning.
	virtual FxInt32 Tell( void ) const;
	/// Sets the current position in the store relative to the beginning.
	virtual void Seek( const FxInt32 pos );
	/// Returns the total length of the store including any unused memory at
	/// the end of the memory block.  If you are embedding FaceFX data into your
	/// own file formats use GetSize().
	virtual FxInt32 Length( void ) const;

	/// Returns a const pointer to the memory.
	const FxByte* GetMemory( void );
	/// Returns the size of the memory in bytes.
	/// \note This isn't necessarily the size of the allocation for the memory
	/// store rather it is the number of bytes currently in use by the memory
	/// store.  If you are embedding FaceFX data into your own file formats
	/// use this rather than Length().
	FxSize GetSize( void );

protected:
	/// Constructor.
	/// \param memory A pointer to the memory to use.
	/// \param size The number of bytes pointed at by \a memory.
	FxArchiveStoreMemoryNoCopy( const FxByte* memory, FxSize size );
	/// Destructor.
	virtual ~FxArchiveStoreMemoryNoCopy();
	/// Destroys the archive store.  This must be overridden to call
	/// the appropriate destructor.
	virtual void Destroy( void );
	/// Returns the size of the archive store class.  This must be 
	/// overridden to return static_cast<FxSize>(sizeof(NewArchiveStoreClass)).
	virtual FxSize GetClassSize( void );

private:
	/// The size of the memory store in bytes.
	FxSize _size;
	/// The amount of memory currently in use in the store in bytes.
	FxSize _inUse;
	/// The memory store.
	const FxByte* _memory;
	/// The current byte offset into the memory store.
	FxSize _currentPos;

	/// Disable copy construction.
	FxArchiveStoreMemoryNoCopy( const FxArchiveStoreMemoryNoCopy& );
	/// Disable assignment.
	FxArchiveStoreMemoryNoCopy& operator=( const FxArchiveStoreMemoryNoCopy& );
};

} // namespace Face

} // namespace OC3Ent

#endif
