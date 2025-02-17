//------------------------------------------------------------------------------
// A very fast file-based archive store.  It loads the entire file into a 
// memory store with one file read and serialization happens from the memory 
// store from that point forward.  Note that the entire file is in memory after
// this operation, so the effect of lazy-loading is zero (that is, all objects
// are stored in memory after this operation even if they haven't been
// "serialized" yet).  This archive store is only used for loading and prevents
// saving.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxArchiveStoreFileFast_H__
#define FxArchiveStoreFileFast_H__

#include "FxPlatform.h"
#include "FxArchiveStoreFile.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare FxArchiveStoreMemoryNoCopy.
class FxArchiveStoreMemoryNoCopy;

/// A very fast archive store for working with files.  This store
/// loads the entire file into memory at once. Lazy loading and saving are
/// not supported with this archive store.
/// Useful for:
/// \li Loading from a file on optical media.
/// \li Loading when speed is a higher priority than memory usage.
/// \ingroup object
class FxArchiveStoreFileFast : public FxArchiveStoreFile
{
public:
	/// Creates an FxArchiveStoreFileFast object.
	/// \param filename The file from which to load.
	/// \param A pointer to a new archive store.
	static FxArchiveStoreFileFast* Create( const FxChar* filename );

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
	/// Returns the total length of the store.
	virtual FxInt32 Length( void ) const;

protected:
	/// Constructor
	/// \param filename The name of the file to load from (or save to).
	FxArchiveStoreFileFast( const FxChar* filename );
	/// Destructor.
	virtual ~FxArchiveStoreFileFast();
	/// Destroys the archive store.  This must be overridden to call
	/// the appropriate destructor.
	virtual void Destroy( void );
	/// Returns the size of the archive store class.  This must be 
	/// overridden to return static_cast<FxSize>(sizeof(NewArchiveStoreClass)).
	virtual FxSize GetClassSize( void );

private:
	/// The raw file in memory.
	FxByte* _pRawFileMemory;
	/// The size (in bytes) of the raw file in memory.
	FxSize _rawFileSize;
	/// The memory store used internally.
	FxArchiveStoreMemoryNoCopy* _pMemoryStore;
	
	/// Disable copy constructor.
	FxArchiveStoreFileFast( const FxArchiveStoreFileFast& );
	/// Disable assignment.
	FxArchiveStoreFileFast& operator=( const FxArchiveStoreFileFast& );
};

} // namespace Face

} // namespace OC3Ent

#endif
