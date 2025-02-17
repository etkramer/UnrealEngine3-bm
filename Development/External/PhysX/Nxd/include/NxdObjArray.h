#ifndef NXD_OBJARRAY_H
#define NXD_OBJARRAY_H

#include <assert.h>
#include "NxdAllocateable.h"

// TODO(dilip): not such great design - would be better to take ptrs than refs.

template<typename T>
class NxdSimpleArray
{
public:
	NxdSimpleArray(): mArray(0), mCapacity(0), mSize(0) {}
	~NxdSimpleArray() 
	{
		if(mArray)
			NXD_FREE(mArray);
	}

	void push_back(T obj)
	{
		if(mSize == mCapacity)
			expand(mCapacity*2+1);

		mArray[mSize++] = obj;
	}

	void resize(NxU32 newSize)
	{
		reserve(newSize);
		mSize = newSize;
	}

	void reserve(NxU32 newSize)
	{
		if(newSize>mCapacity)
		{
			NxU32 s = mCapacity;
			do
				s = s*2+1;
			while(s<newSize);

			expand(s);
		}
	}

	NxU32 size() const
	{
		return mSize;
	}

	T &operator[](NxU32 i)
	{
		assert(i<mSize);
		return mArray[i];
	}

	const T &operator[](NxU32 i) const
	{
		assert(i<mSize);
		return mArray[i];
	}


	void clear()
	{
		mSize = 0;
	}

	void replaceWithLast(NxU32 index)
	{
		assert(index<mSize);
		mArray[index] = mArray[--mSize];
	}

	T *front() const 
	{
		return mArray;
	}

private:
	void expand(NxU32 newSize)
	{
		T *store = static_cast<T *>(NXD_MALLOC(newSize * sizeof(T)));
		memcpy(store,mArray,mSize*sizeof(T));
		NXD_FREE(mArray);
		mArray = store;
		mCapacity = newSize;
	}

	T *				mArray;
	NxU32			mSize;
	NxU32			mCapacity;
};



template <class Object, class Indexer>
class NxdObjArray
{
public:
	static const NxU32 NO_INDEX = 0xffffffff;

	bool contains(Object *obj) const
	{
		return Indexer::index(*obj)!=NO_INDEX;
	}

	void push_back(Object *obj)
	{
		assert(!contains(obj));
		Indexer::index(*obj) = mArray.size();
		mArray.push_back(obj);
	}

	void remove(Object *obj)
	{
		assert(contains(obj));
		NxU32 &index = Indexer::index(*obj);
		mArray.replaceWithLast(index);
		if(index!=mArray.size())
			Indexer::index(*mArray[index]) = index;

		index = NO_INDEX;
	}

	NxU32 size() const
	{
		return mArray.size();
	}

	Object *operator[](NxU32 i)
	{
		return mArray[i];
	}

	void clear()
	{
		for(NxU32 i=0;i<mArray.size();i++)
			Indexer::index(*mArray[i]) = NO_INDEX;
		mArray.clear();
	}

	Object **front() const
	{
		return mArray.front();
	}

private:
	NxdSimpleArray<Object *> mArray;
};

#endif

