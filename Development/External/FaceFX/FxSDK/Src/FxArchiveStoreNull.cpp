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

#include "FxArchiveStoreNull.h"

namespace OC3Ent
{

namespace Face
{

FxArchiveStoreNull* FxArchiveStoreNull::Create( void )
{
	return new FxArchiveStoreNull();
}

FxBool FxArchiveStoreNull::IsValid( void ) const
{
	return FxTrue;
}

FxBool FxArchiveStoreNull::OpenRead( void )
{
	return FxTrue;
}

FxBool FxArchiveStoreNull::OpenWrite( void )
{
	return FxTrue;
}

FxBool FxArchiveStoreNull::Close( void )
{
	return FxTrue;
}

FxBool FxArchiveStoreNull::Read( FxByte* FxUnused(data), FxSize FxUnused(size) )
{
	return FxTrue;
}

FxBool FxArchiveStoreNull::Write( const FxByte* FxUnused(data), FxSize size )
{
	_numBytes += size;
	return FxTrue;
}

FxInt32 FxArchiveStoreNull::Tell( void ) const
{
	return 0;
}

void FxArchiveStoreNull::Seek( const FxInt32 FxUnused(pos) )
{
}

FxInt32 FxArchiveStoreNull::Length( void ) const
{
	return 0;
}

FxSize FxArchiveStoreNull::GetSize( void ) const
{
	return _numBytes;
}

void FxArchiveStoreNull::Destroy( void )
{
	this->~FxArchiveStoreNull();
}

FxSize FxArchiveStoreNull::GetClassSize( void )
{
	return static_cast<FxSize>(sizeof(FxArchiveStoreNull));
}

} // namespace Face

} // namespace OC3Ent
