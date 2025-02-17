//------------------------------------------------------------------------------
// A memory-based archive store.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxArchiveStoreMemory.h"
#include "FxMemory.h"

namespace OC3Ent
{

namespace Face
{

#define kInitialMemoryStoreSize  1024
#define kMemoryStoreSizeMultiple 2

FxArchiveStoreMemory* 
FxArchiveStoreMemory::Create( const FxByte* memory, FxSize size )
{
	return new FxArchiveStoreMemory(memory, size);
}

FxBool FxArchiveStoreMemory::IsValid( void ) const
{
	if( _memory && _size > 0 )
	{
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxArchiveStoreMemory::OpenRead( void )
{
	if( !_memory || _size == 0 )
	{
		return FxFalse;
	}
	return FxTrue;
}

FxBool FxArchiveStoreMemory::OpenWrite( void )
{
	if( _memory || _size > 0 )
	{
		return FxFalse;
	}
	_size = kInitialMemoryStoreSize;
	_memory = static_cast<FxByte*>(FxAlloc(_size * sizeof(FxByte), "FxArchiveStoreMemory::OpenWrite()"));
	return FxTrue;
}

FxBool FxArchiveStoreMemory::Close( void )
{
	if( _memory )
	{
		FxFree(_memory, _size * sizeof(FxByte));
		_memory = NULL;
	}
	return FxTrue;
}

FxBool FxArchiveStoreMemory::Read( FxByte* data, FxSize size )
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

FxBool FxArchiveStoreMemory::Write( const FxByte* data, FxSize size )
{
	if( !_memory )
	{
		return FxFalse;
	}
	if( size > _size - _currentPos )
	{
		FxSize oldSize = _size;
		while( _size - _currentPos < size )
		{
			_size *= kMemoryStoreSizeMultiple;
		}
		FxByte* newMemory = static_cast<FxByte*>(FxAlloc(_size * sizeof(FxByte), "FxArchiveStoreMemory::Write"));
		FxMemcpy(newMemory, _memory, oldSize);
		FxFree(_memory, oldSize * sizeof(FxByte));
		_memory = newMemory;
	}
	FxMemcpy(_memory + _currentPos, data, size);
	_currentPos += size;
	_inUse += size;
	return FxTrue;
}

FxInt32 FxArchiveStoreMemory::Tell( void ) const
{
	return static_cast<FxInt32>(_currentPos);
}

void FxArchiveStoreMemory::Seek( const FxInt32 pos )
{
	FxAssert(static_cast<FxSize>(pos) < _size);
	_currentPos = static_cast<FxSize>(pos);
}

FxInt32 FxArchiveStoreMemory::Length( void ) const
{
	return _size;
}

const FxByte* FxArchiveStoreMemory::GetMemory( void )
{
	return _memory;
}

FxSize FxArchiveStoreMemory::GetSize( void )
{
	return _inUse;
}

FxArchiveStoreMemory::FxArchiveStoreMemory( const FxByte* memory, FxSize size )
	: _size(size)
	, _inUse(size)
	, _memory(NULL)
	, _currentPos(0)
{
	if( memory && size > 0 )
	{
		_memory = static_cast<FxByte*>(FxAlloc(_size * sizeof(FxByte), "FxArchiveStoreMemory"));
		FxMemcpy(_memory, memory, _size);
	}
	else
	{
		_size  = 0;
		_inUse = 0;
	}
}

FxArchiveStoreMemory::~FxArchiveStoreMemory()
{
	Close();
}

void FxArchiveStoreMemory::Destroy( void )
{
	this->~FxArchiveStoreMemory();
}

FxSize FxArchiveStoreMemory::GetClassSize( void )
{
	return static_cast<FxSize>(sizeof(FxArchiveStoreMemory));
}

} // namespace Face

} // namespace OC3Ent
