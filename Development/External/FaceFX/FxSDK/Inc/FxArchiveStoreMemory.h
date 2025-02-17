//------------------------------------------------------------------------------
// A memory-based archive store.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxArchiveStoreMemory_H__
#define FxArchiveStoreMemory_H__

#include "FxPlatform.h"
#include "FxArchiveStore.h"

namespace OC3Ent
{

namespace Face
{

/// An archive store for working in memory.
/// This archive store serializes to/from a memory chunk, rather than a file.
/// Useful for:
/// \li Saving to a embeddable format.
/// \ingroup object
class FxArchiveStoreMemory : public FxArchiveStore
{
public:
	/// Creates an FxArchiveStoreMemory object.
	/// When using the archive store to load from a chunk in memory, both \a memory
	/// and \a size passed to FxArchiveStoreMemory::Create must be valid.
	/// When using the archive store to save to a chunk in memory, pass \t NULL
	/// for \a memory and \t 0 for \a size.
	/// \param memory A pointer to the memory to load from, or NULL if saving.
	/// \param size The length of the chunk pointed to by \a memory in bytes, or 0 if saving.
	/// \return A pointer to a new archive store.
	/// \note The archive store does not take control of \a memory, so the client
	/// is still responsible for freeing its memory.  A copy of the memory will
	/// be created internally.
	static FxArchiveStoreMemory* Create( const FxByte* memory, FxSize size );

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
	/// Note that the archive store creates an internal copy of the memory,
	/// so the client is still responsible for deleting the pointer passed
	/// to the FxArchiveStoreMemory constructor.
	FxArchiveStoreMemory( const FxByte* memory, FxSize size );
	/// Destructor.
	virtual ~FxArchiveStoreMemory();
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
	FxByte* _memory;
	/// The current byte offset into the memory store.
	FxSize _currentPos;
	
	/// Disable copy construction.
	FxArchiveStoreMemory( const FxArchiveStoreMemory& );
	/// Disable assignment.
	FxArchiveStoreMemory& operator=( const FxArchiveStoreMemory& );
};

} // namespace Face

} // namespace OC3Ent

#endif
