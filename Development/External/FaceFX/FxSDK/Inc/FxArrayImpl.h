//------------------------------------------------------------------------------
// A dynamic array.
// Do not include this file.  Include FxArray.h instead.
//
// Owner: John Briggs
// 
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxTypeTraits.h"
#include "FxArrayBase.h"

namespace OC3Ent
{

namespace Face
{

// Default constructor.
template<typename Elem>
FxArray<Elem>::FxArray(FxSize numReserved)
	: _container(numReserved)
{
}

// Construct an array from a native array.
template<typename Elem>
FxArray<Elem>::FxArray(const value_type* pArray, FxSize numElements)
	: _container(numElements)
{
	if( pArray )
	{
		while( _container._usedCount < numElements )
		{
			FxConstruct(_container._v + _container._usedCount,
				pArray[_container._usedCount]);
			++_container._usedCount;
		}
	}
}

// Copy constructor.
template<typename Elem>
FxArray<Elem>::FxArray(const FxArray<Elem>& other)
	: _container(other._container._usedCount)
{
	while( _container._usedCount < other._container._usedCount )
	{
		FxConstruct(_container._v + _container._usedCount, 
			other._container._v[_container._usedCount]);
		++_container._usedCount;
	}
}

// Assignment operator.
template<typename Elem>
FxArray<Elem>& FxArray<Elem>::operator=(const FxArray<Elem>& other)
{
	_array_type temp(other);
	_container.Swap(temp._container);
	return *this;
}

// Clears the array and deallocates its memory.
template<typename Elem>
void FxArray<Elem>::Clear()
{
	_array_type().Swap(*this);
}

// Returns the number of elements in the array.
template<typename Elem> FX_INLINE
FxSize FxArray<Elem>::Length() const
{
	return _container._usedCount;
}

// Returns the number of elements allocated.
template<typename Elem> FX_INLINE
FxSize FxArray<Elem>::Allocated() const
{
	return _container._allocatedCount;
}

// Returns true if the array has no elements.
template<typename Elem> FX_INLINE
FxBool FxArray<Elem>::IsEmpty() const
{
	return _container._usedCount == 0;
}

// Element access.
template<typename Elem> FX_INLINE
Elem& FxArray<Elem>::operator[](FxSize index)
{
	FxAssert(index < _container._usedCount);
	return _container._v[index];
}

template<typename Elem> FX_INLINE
const Elem& FxArray<Elem>::operator[](FxSize index) const
{
	FxAssert(index < _container._usedCount);
	return _container._v[index];
}

// Returns the beginning of the array.
template<typename Elem> FX_INLINE
typename FxArray<Elem>::Iterator FxArray<Elem>::Begin()
{
	return _container._v;
}

template<typename Elem> FX_INLINE
typename FxArray<Elem>::ConstIterator FxArray<Elem>::Begin() const
{
	return _container._v;
}

// Returns the end of the array.
template<typename Elem> FX_INLINE
typename FxArray<Elem>::Iterator FxArray<Elem>::End()
{
	return _container._v + _container._usedCount;
}

template<typename Elem> FX_INLINE
typename FxArray<Elem>::ConstIterator FxArray<Elem>::End() const
{
	return _container._v + _container._usedCount;
}

// Reserves room for n elements.
template<typename Elem>
void FxArray<Elem>::Reserve(FxSize n)
{
	if( n > _container._allocatedCount )
	{
		_container_type temp(n);
		while( temp._usedCount < _container._usedCount )
		{
			FxConstruct(temp._v + temp._usedCount, _container._v[temp._usedCount]);
			++temp._usedCount;
		}
		_container.Swap(temp);
	}
}

// Adds an element to the end of the array.
template<typename Elem>
void FxArray<Elem>::PushBack(const Elem& element)
{
	if( _container._usedCount >= _container._allocatedCount )
	{
		// Grow the array, if necessary.
		_container_type temp(_container._allocatedCount == 0 ? 1 : _container._allocatedCount * 2);
		while( temp._usedCount < _container._usedCount )
		{
			FxConstruct(temp._v + temp._usedCount, _container._v[temp._usedCount]);
			++temp._usedCount;
		}
		FxConstruct(temp._v + temp._usedCount, element);
		++temp._usedCount;
		_container.Swap(temp);
	}
	else
	{
		// Otherwise, just add the element to the back of the array.
		FxConstruct(_container._v + _container._usedCount, element);
		++_container._usedCount;
	}
}

// Removes the last element in the array.
template<typename Elem>
void FxArray<Elem>::PopBack()
{
	FxAssert(_container._usedCount > 0);
	--_container._usedCount;
	FxDestroy(_container._v + _container._usedCount);
	// Zero the memory to prevent unintentional accesses.
	FxMemset(_container._v + _container._usedCount, 0, sizeof(Elem));
}

// Returns a reference to the last element in the array.
template<typename Elem> FX_INLINE
Elem& FxArray<Elem>::Back()
{
	FxAssert(_container._usedCount > 0);
	return _container._v[_container._usedCount-1];
}

// Returns a const reference to the last element in the array.
template<typename Elem> FX_INLINE
const Elem& FxArray<Elem>::Back() const
{
	FxAssert(_container._usedCount > 0);
	return _container._v[_container._usedCount-1];
}

// Returns a reference to the front element in the array.
template<typename Elem> FX_INLINE
Elem& FxArray<Elem>::Front()
{
	FxAssert(_container._usedCount > 0);
	return _container._v[0];
}

// Returns a const reference to the front element in the array.
template<typename Elem> FX_INLINE
const Elem& FxArray<Elem>::Front() const
{
	FxAssert(_container._usedCount > 0);
	return _container._v[0];
}

// Inserts an element at index.
template<typename Elem>
void FxArray<Elem>::Insert(const Elem& element, FxSize index)
{
	FxAssert(index <= _container._usedCount);
	if(_container._usedCount >= _container._allocatedCount)
	{
		// Resize the container to hold the new item.
		_container_type temp(_container._allocatedCount + 1);
		// Copy elements up to the one to be inserted.
		while( temp._usedCount < index )
		{
			FxConstruct(temp._v + temp._usedCount, _container._v[temp._usedCount]);
			++temp._usedCount;
		}
		// Insert element.
		FxConstruct(temp._v + temp._usedCount, element);
		++temp._usedCount;
		// Copy elements after the inserted.
		while( temp._usedCount <= _container._usedCount )
		{
			FxConstruct(temp._v + temp._usedCount, _container._v[temp._usedCount-1]);
			++temp._usedCount;
		}
		// Take ownership of the new array.
		_container.Swap(temp);
	}
	else
	{
		if( index != _container._usedCount )
		{
			// Make room for the item.
			if( FxTypeTraits::has_trivial_assign<value_type>::value )
			{
				// Copy the entire chunk at once.
				FxMemmove(_container._v + index + 1, _container._v + index, sizeof(value_type) * (_container._usedCount - index));
			}
			else
			{
				// We need to assign one-by-one.
				value_type* curr = _container._v + _container._usedCount;
				// Construct the first (technically, the last...) element.
				FxConstruct(curr, *(curr-1));
				curr -= 1;
				// Assign the rest.
				while( curr != _container._v + index )
				{
					*curr = *(curr-1);
					curr -= 1;
				}
			}
			// Make the new item.
			_container._v[index] = element;
			++_container._usedCount;
		}
		else
		{
			// Degenerate case - essentially a pushback.
			FxConstruct(_container._v + index, element);
			++_container._usedCount;
		}
	}
}

// Removes the element at index.
template<typename Elem>
void FxArray<Elem>::Remove(FxSize index)
{
	FxAssert(index < _container._usedCount);
	if( FxTypeTraits::has_trivial_assign<value_type>::value )
	{
		// Copy the entire chunk at once.
		FxMemmove(_container._v + index, _container._v + index + 1, sizeof(value_type) * (_container._usedCount - index - 1));
	}
	else
	{
		// Copy the items individually.
		value_type* curr = _container._v + index;
		while( curr != _container._v + _container._usedCount - 1 )
		{
			(*curr) = *(curr + 1);
			curr += 1;
		}
	}

	// Clean up the last item.
	--_container._usedCount;
	FxDestroy(_container._v + _container._usedCount);
	// Zero the memory to prevent unintentional accesses.
	FxMemset(_container._v + _container._usedCount, 0, sizeof(Elem));
}

// Removes the element at the iterator.
template<typename Elem>
void FxArray<Elem>::RemoveIterator(Iterator iter)
{
	Remove(static_cast<FxSize>(iter - _container._v));
}

// Finds the index of element.
template<typename Elem>
FxSize FxArray<Elem>::Find(const value_type& element, FxSize startIndex) const
{
	FxAssert(startIndex <= _container._usedCount);
	for( FxSize i = startIndex; i < _container._usedCount; ++i )
	{
		if( element == _container._v[i] )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

// Swaps contents with an other array.
template<typename Elem>
void FxArray<Elem>::Swap(FxArray& other)
{
	_container.Swap(other._container);
}

template<typename Elem> FX_INLINE
void FxArray<Elem>::Prefetch() const
{
	_container.Prefetch();
}

template<typename Elem> FX_INLINE
Elem* FxArray<Elem>::GetData() const
{
	return _container._v;
}

template<typename Elem>
void FxArray<Elem>::SwapArrayBase(_container_type& other)
{
	_container.Swap(other);
}

// Array equality.  Arrays are equal if their lengths are equal and each
// element is equal to the corresponding element in the other array.
template<typename T>
FxBool operator==(const FxArray<T>& lhs, const FxArray<T>& rhs)
{
	if( lhs.Length() == rhs.Length() )
	{
		FxSize length = lhs.Length();
		for( FxSize i = 0; i < length; ++i )
		{
			if( !(lhs[i] == rhs[i]) )
			{
				return FxFalse;
			}
		}
		return FxTrue;
	}
	return FxFalse;
}

// Array inequality.
template<typename T>
FxBool operator!=(const FxArray<T>& lhs, const FxArray<T>& rhs)
{
	return !operator==(lhs, rhs);
}

template<typename Elem>
FX_INLINE FxArchive& operator<<( FxArchive& arc, FxArray<Elem>& array )
{
	return array.Serialize(arc);
}

} // namespace Face

} // namespace OC3Ent
