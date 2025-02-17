//------------------------------------------------------------------------------
// This class implements a circular doubly-linked list.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxList_H__
#define FxList_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxMemory.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxListVersion 0

/// A templated doubly-linked list.
/// \ingroup support
template<typename FxListElem> class FxList
{
private:
	// Forward declare FxListNode.
	class FxListNode;
public:
	// Forward declare Iterator.
	class Iterator;
	// Forward declare ConstIterator.
	class ConstIterator;
	// Forward declare ReverseIterator.
	class ReverseIterator;

	/// Default constructor.
	FxList();
	/// Copy constructor.
	FxList( const FxList& other );
	/// Assignment operator.
	FxList& operator=( const FxList& other );
	/// Destructor.
	~FxList();

	/// Returns an iterator to the start of the list.
	Iterator Begin( void );
	/// Returns a const iterator to the start of the list.
	ConstIterator Begin( void ) const;
	/// Returns an iterator to the end of the list.
	Iterator End( void );
	/// Returns a const iterator to the end of the list.
	ConstIterator End( void ) const;
	/// Returns an iterator to the item that equals \a toFind, or End() if it was
	/// not found.
	Iterator Find( const FxListElem& toFind );
	/// Returns a const iterator to the item that equals \a toFind, or End() if it was 
	// not found.
	ConstIterator Find( const FxListElem& toFind ) const;

	/// Returns the start of reverse iteration.
	ReverseIterator ReverseBegin( void ) const;
	/// Returns the end of reverse iterator.
	ReverseIterator ReverseEnd( void ) const;

	/// Clears the list.
	void Clear( void );
	/// Returns FxTrue if the list is empty.
	FxBool IsEmpty( void ) const;
	/// Returns the number of elements in the list.
	FxSize Length( void ) const;
	/// Returns the amount of memory occupied by the list.
	FxSize Allocated( void ) const;

	/// Returns the element at the back of the list.
	FxListElem& Back( void );
	/// Returns a const reference to the element at the back of the list.
	const FxListElem& Back( void ) const;
	/// Adds element to the back of the list.
	void PushBack( const FxListElem& element );
	/// Removes the element from the back of the list.
	void PopBack( void );
	/// Returns the element at the front of the list.
	FxListElem& Front( void );
	/// Returns a const reference to the element at the front of the list.
	const FxListElem& Front( void ) const;
	/// Adds element to the front of the list.
	void PushFront( const FxListElem& element );
	/// Removes element from the front of the list.
	void PopFront( void );

	/// Inserts the element before the iterator
	Iterator Insert( const FxListElem& element, Iterator iter );
	/// Removes the element at the iterator.
	void Remove( Iterator iter );
	/// Removes the element at the iterator
	void RemoveIterator( Iterator iter );

	/// Removes all elements equal to a given value.
	void RemoveIfEqual( const FxListElem& element );

	/// Sorts the list from smallest to largest. Assumes the contained object 
	/// defines operator<(), and will not compile if the contained object 
	/// doesn't supply that definition.
	void Sort( void );

	/// A list iterator.
	/// \ingroup support
	class Iterator
	{
	public:
		/// Constructor.
		Iterator( FxListNode* pNode = NULL )
			: _pNode(pNode)
		{
		}

		/// Advance the iterator.
		Iterator& operator++( void )
		{
			_pNode = _pNode->next;
			return *this;
		}

		/// Move the iterator backwards.
		Iterator& operator--( void )
		{
			_pNode = _pNode->prev;
			return *this;
		}

		/// Dereference the iterator.
		FxListElem& operator*( void )
		{
			FxAssert( _pNode );
			return _pNode->element;
		}

		/// Return a pointer to the element.
		FxListElem* operator->( void )
		{
			FxAssert( _pNode );
			return &(_pNode->element);
		}

		/// Tests for iterator equality.
		FxBool operator==( const Iterator& other )
		{
			return _pNode == other._pNode;
		}

		/// Tests for iterator inequality.
		FxBool operator!=( const Iterator& other )
		{
			return _pNode != other._pNode;
		}

		/// Advances the iterator \a num times.
		Iterator& Advance( FxSize num )
		{
			while( num-- )
			{
				_pNode = _pNode->next;
			}
			return* this;
		}

		/// Returns the node that the iterator corresponds to.
		FxListNode* GetNode( void ) { return _pNode; }

	private:
		FxListNode* _pNode;
	};

	/// A const list iterator.
	/// \ingroup support
	class ConstIterator
	{
	public:
		/// Constructor.
		ConstIterator( FxListNode* pNode = NULL )
			: _pNode(pNode)
		{
		}

		/// Advance the iterator.
		ConstIterator& operator++( void )
		{
			_pNode = _pNode->next;
			return *this;
		}

		/// Move the iterator backwards.
		ConstIterator& operator--( void )
		{
			_pNode = _pNode->prev;
			return *this;
		}

		/// Dereference the iterator.
		const FxListElem& operator*( void )
		{
			FxAssert( _pNode );
			return _pNode->element;
		}

		/// Return a pointer to the element.
		const FxListElem* operator->( void )
		{
			FxAssert( _pNode );
			return &(_pNode->element);
		}

		/// Tests for iterator equality.
		FxBool operator==( const ConstIterator& other )
		{
			return _pNode == other._pNode;
		}

		/// Tests for iterator inequality.
		FxBool operator!=( const ConstIterator& other )
		{
			return _pNode != other._pNode;
		}

		/// Advances the iterator \a num times.
		ConstIterator& Advance( FxSize num )
		{
			while( num-- )
			{
				_pNode = _pNode->next;
			}
			return* this;
		}

		/// Returns the node that the iterator corresponds to.
		const FxListNode* GetNode( void ) { return _pNode; }

	private:
		const FxListNode* _pNode;
	};

	/// A list reverse iterator.
	/// \ingroup support
	class ReverseIterator
	{
	public:
		/// Constructor.
		ReverseIterator( FxListNode* pNode = NULL )
			: _pNode(pNode)
		{
		}

		/// Advance the iterator.
		ReverseIterator& operator++( void )
		{
			_pNode = _pNode->prev;
			return *this;
		}

		/// Move the iterator backwards.
		ReverseIterator& operator--( void )
		{
			_pNode = _pNode->next;
			return *this;
		}

		/// Dereference the iterator.
		FxListElem& operator*( void )
		{
			FxAssert( _pNode );
			return _pNode->element;
		}

		/// Return a pointer to the element.
		FxListElem* operator->( void )
		{
			FxAssert( _pNode );
			return &(_pNode->element);
		}

		/// Tests for iterator equality.
		FxBool operator==( const ReverseIterator& other )
		{
			return _pNode == other._pNode;
		}

		/// Tests for iterator inequality.
		FxBool operator!=( const ReverseIterator& other )
		{
			return _pNode != other._pNode;
		}

		/// Advances the iterator \a num times.
		ReverseIterator& Advance( FxSize num )
		{
			while( num-- )
			{
				_pNode = _pNode->prev;
			}
			return* this;
		}

		/// Returns the node that the iterator corresponds to.
		FxListNode* GetNode( void ) { return _pNode->prev; }

	private:
		FxListNode* _pNode;	
	};

	/// Serializes the list to an archive.
	FxArchive& Serialize( FxArchive& arc )
	{
		// As of version 1.7, container classes have the same version as the 
		// SDK itself so there is no need to explicitly serialize it.  This eats
		// the version when loading pre-1.7 archives.
		if( arc.GetSDKVersion() < 1700 && arc.IsLoading() )
		{
			FxUInt16 version = 0;
			arc << version;
		}

		FxSize length = Length();
		arc << length;

		if( arc.IsSaving() )
		{
#ifndef NO_SAVE_VERSION
			Iterator curr = Begin();
			Iterator end  = End();
			for( ; curr != end; ++curr )
			{
				arc << *curr;
			}
#endif
		}
		else
		{
			Clear();
			for( FxSize i = 0; i < length; ++i )
			{
				FxListElem element;
				PushBack(element);
				arc << _head->prev->element;
			}
		}

		return arc;
	}

private:
	class FxListNode
	{
	public:
		FxListNode()
		{
			next = NULL;
			prev = NULL;
		}
		FxListNode( const FxListElem& iElement, 
					FxListNode* iNext, FxListNode* iPrev )
			: element(iElement)
			, next(iNext)
			, prev(iPrev)
		{
		}

		FxListElem  element;
		FxListNode* next;
		FxListNode* prev;
	};

	mutable FxListNode* _head;
	FxSize _size;
};

template<typename FxListElem>
FX_INLINE FxArchive& operator<<( FxArchive& arc, FxList<FxListElem>& list )
{
	return list.Serialize(arc);
}

} // namespace Face

} // namespace OC3Ent

// Include the implementation
#include "FxListImpl.h"

#endif
