//------------------------------------------------------------------------------
// A memory-based archive store that does not make an internal copy of the
// memory passed in.  This archive store can only be used for loading and 
// prevents saving.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxArchiveStoreMemoryNoCopy.h"
#include "FxMemory.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

FxArchiveStoreMemoryNoCopy* 
FxArchiveStoreMemoryNoCopy::Create( const FxByte* memory, FxSize size )
{
	return new FxArchiveStoreMemoryNoCopy(memory, size);
}

FxBool FxArchiveStoreMemoryNoCopy::IsValid( void ) const
{
	if( _memory && _size > 0 )
	{
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxArchiveStoreMemoryNoCopy::OpenRead( void )
{
	if( !_memory || _size == 0 )
	{
		return FxFalse;
	}
	return FxTrue;
}

FxBool FxArchiveStoreMemoryNoCopy::OpenWrite( void )
{
	return FxFalse;
}

FxBool FxArchiveStoreMemoryNoCopy::Close( void )
{
	return FxTrue;
}

FxBool FxArchiveStoreMemoryNoCopy::Read( FxByte* data, FxSize size )
{
	FxAssert(size <= _size - _currentPos);
	if( !_memory || size > _size - _currentPos )
	{
		return FxFalse;
	}
	FxMemcpy(data, _memory + _currentPos, size);
	_currentPos += size;
	return FxTrue;
}

FxBool 
FxArchiveStoreMemoryNoCopy::Write( const FxByte* FxUnused(data), FxSize FxUnused(size) )
{
	FxAssert(!"Attempt to call FxArchiveStoreMemoryNoCopy::Write()!");
	return FxFalse;
}

FxInt32 FxArchiveStoreMemoryNoCopy::Tell( void ) const
{
	return static_cast<FxInt32>(_currentPos);
}

void FxArchiveStoreMemoryNoCopy::Seek( const FxInt32 pos )
{
	FxAssert(static_cast<FxSize>(pos) < _size);
	_currentPos = static_cast<FxSize>(pos);
}

FxInt32 FxArchiveStoreMemoryNoCopy::Length( void ) const
{
	return _size;
}

const FxByte* FxArchiveStoreMemoryNoCopy::GetMemory( void )
{
	return _memory;
}

FxSize FxArchiveStoreMemoryNoCopy::GetSize( void )
{
	return _inUse;
}

FxArchiveStoreMemoryNoCopy::FxArchiveStoreMemoryNoCopy( const FxByte* memory, FxSize size )
	: _size(size)
	, _inUse(size)
	, _memory(NULL)
	, _currentPos(0)
{
	if( memory && size > 0 )
	{
		_memory = memory;
	}
	else
	{
		_size  = 0;
		_inUse = 0;
	}
}

FxArchiveStoreMemoryNoCopy::~FxArchiveStoreMemoryNoCopy()
{
	Close();
}

void FxArchiveStoreMemoryNoCopy::Destroy( void )
{
	this->~FxArchiveStoreMemoryNoCopy();
}

FxSize FxArchiveStoreMemoryNoCopy::GetClassSize( void )
{
	return static_cast<FxSize>(sizeof(FxArchiveStoreMemoryNoCopy));
}

} // namespace Face

} // namespace OC3Ent
