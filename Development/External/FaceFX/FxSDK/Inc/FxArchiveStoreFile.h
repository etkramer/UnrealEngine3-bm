//------------------------------------------------------------------------------
// A file-based archive store.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxArchiveStoreFile_H__
#define FxArchiveStoreFile_H__

#include "FxPlatform.h"
#include "FxArchiveStore.h"
#include "FxString.h"
#include "FxFile.h"

namespace OC3Ent
{

namespace Face
{

/// An archive store for working with files.
/// This archive store passes the read and write requests directly through to
/// the underlying file.  No memory buffering is performed.
/// Useful for:
/// \li Loading from a file on non-optical media when speed is not an issue, or
/// memory is limited.
/// \li Saving to a file.
/// \sa \ref loadActor "Loading an Actor"
/// \ingroup object
class FxArchiveStoreFile : public FxArchiveStore
{
public:
	/// Creates an FxArchiveStoreFile object.
	/// \param filename The file from which to load.
	/// \return A pointer to a new archive store.
	static FxArchiveStoreFile* Create( const FxChar* filename );

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
	FxArchiveStoreFile( const FxChar* filename );
	/// Destructor.
	virtual ~FxArchiveStoreFile();
	/// Destroys the archive store.  This must be overridden to call
	/// the appropriate destructor.
	virtual void Destroy( void );
	/// Returns the size of the archive store class.  This must be 
	/// overridden to return static_cast<FxSize>(sizeof(NewArchiveStoreClass)).
	virtual FxSize GetClassSize( void );

private:
	/// The file name associated with the store.
	FxString _filename;
	/// The file pointer associated with the store.
	FxFile _file;

	/// Disable copy constructor.
	FxArchiveStoreFile( const FxArchiveStoreFile& );
	/// Disable assignment.
	FxArchiveStoreFile& operator=( const FxArchiveStoreFile& );
};

} // namespace Face

} // namespace OC3Ent

#endif
