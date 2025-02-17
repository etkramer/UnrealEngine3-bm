//------------------------------------------------------------------------------
// A 'NULL' archive store.  Methods will always succeed but the archive store
// does nothing internally besides keep track of the number of bytes written to
// it.  It is meant to be used with archives in AM_CreateDirectory or 
// AM_ApproximateMemoryUsage mode only.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxArchiveStoreNull_H__
#define FxArchiveStoreNull_H__

#include "FxPlatform.h"
#include "FxArchiveStore.h"

namespace OC3Ent
{

namespace Face
{

/// A 'NULL' archive store.  Methods will always succeed but the archive store
/// does nothing internally besides keep track of the number of bytes written to
/// it.  It is meant to be used with archives in AM_CreateDirectory or 
/// AM_ApproximateMemoryUsage mode only.
/// \ingroup object
class FxArchiveStoreNull : public FxArchiveStore
{
public:
	/// Creates an FxArchiveStoreNull object.
	/// \return A pointer to a new archive store.
	static FxArchiveStoreNull* Create( void );

	/// Returns FxTrue if the archive store is in a valid state.
	/// This always returns FxTrue.
	virtual FxBool IsValid( void ) const;
	/// Opens the archive store for reading.
	/// This always returns FxTrue.
	virtual FxBool OpenRead( void );
	/// Opens the archive store for writing.
	/// This always returns FxTrue.
	virtual FxBool OpenWrite( void );
	/// Closes the archive store.
	/// This always returns FxTrue.
	virtual FxBool Close( void );
	/// Reads size bytes from the archive store and puts them in data.
	/// This always returns FxTrue.
	virtual FxBool Read( FxByte* data, FxSize size );
	/// Writes size bytes from data into the archive store.
	/// This always returns FxTrue.
	virtual FxBool Write( const FxByte* data, FxSize size );
	/// Returns the current position in the store relative to the beginning.
	/// This always returns 0.
	virtual FxInt32 Tell( void ) const;
	/// Sets the current position in the store relative to the beginning.
	virtual void Seek( const FxInt32 pos );
	/// Returns the total length of the store.  This always returns 0.
	virtual FxInt32 Length( void ) const;

	/// Returns the total number of bytes written to the store.  This is useful
	/// for approximating memory usage of objects.
	FxSize GetSize( void ) const;

protected:
	/// Constructor.
	FxArchiveStoreNull() : _numBytes(0) {}
	/// Destructor.
	virtual ~FxArchiveStoreNull() {}
	/// Destroys the archive store.  This must be overridden to call
	/// the appropriate destructor.
	virtual void Destroy( void );
	/// Returns the size of the archive store class.  This must be 
	/// overridden to return static_cast<FxSize>(sizeof(NewArchiveStoreClass)).
	virtual FxSize GetClassSize( void );

	/// Holds the total number of bytes written to the archive.
	FxSize _numBytes;
	
private:
	/// Disable copy construction.
	FxArchiveStoreNull( const FxArchiveStoreNull& );
	/// Disable assignment.
	FxArchiveStoreNull& operator=( const FxArchiveStoreNull& );
};

} // namespace Face

} // namespace OC3Ent

#endif
