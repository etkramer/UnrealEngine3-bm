#ifndef NX_DOUBLEBUFFER_NXD_PAIRFLAGHASH
#define NX_DOUBLEBUFFER_NXD_PAIRFLAGHASH

#include <assert.h>
#include "NxdObjArray.h"


template <typename T>
struct NxdPairFlagHashEntry
{
	T t0; 
	T t1;
	NxU32 flags;
	NxU32 next;

	NxdPairFlagHashEntry(T v0, T v1, NxU32 f, NxU32 n): flags(f), next(n), t0(v0), t1(v1) 
	{
		assert(t0<t1);
	}
};


template <typename T>
class NxdPairFlagHash
{
public:

	NxU32 operator()(void *a, void *b)
	{
		return operator()(size_t(a)<<16 ^ size_t(a)>>2);
	}

	NxdPairFlagHash()
	{
		clear();
	}

	void clear()
	{
		for(NxU32 i=0;i<HASH_SIZE;i++) mHash[i] = NOT_FOUND;
		mEntries.clear();
	}

	NxdPairFlagHashEntry<T> *front()	{	return mEntries.front();	}
	NxU32 size()						{	return mEntries.size();		}

	NxU32 hash(void *p) const
	{
		NxU32 k = NxU32(size_t(p));
		// Thomas Wang's 32 bit mix
		// http://www.cris.com/~Ttwang/tech/inthash.htm

		k += ~(k << 15);
		k ^= (k >> 10);
		k += (k << 3);
		k ^= (k >> 6);
		k += ~(k << 11);
		k ^= (k >> 16);
		return k;
	}

	NxU32 key(void *t0, void *t1) const { return (hash(t0)^hash(t1)) % HASH_SIZE; }

	bool get(T t0, T t1, NxU32 &flags) const
	{
		if(t1<t0) { T x = t0; t0 = t1; t1 = x; }

		for(NxU32 i = mHash[key(t0,t1)]; i!=NOT_FOUND; i = mEntries[i].next)
		{
			const NxdPairFlagHashEntry <T> &entry = mEntries[i];
			if(t0 == entry.t0 && t1 == entry.t1)
			{
				flags = entry.flags;
				return true;
			}
		}
		return false;
	}

	void set(T t0, T t1, NxU32 flags)
	{
		if(t1<t0) { T x = t0; t0 = t1; t1 = x; }

		NxU32 k = key(t0,t1);
		for(NxU32 i = mHash[k]; i!=NOT_FOUND; i = mEntries[i].next)
		{
			NxdPairFlagHashEntry <T> &entry = mEntries[i];
			if(t0 == entry.t0 && t1 == entry.t1)
			{
				entry.flags = flags;
				return;
			}
		}
		NxU32 s = mEntries.size();
		mEntries.push_back(NxdPairFlagHashEntry<T>(t0,t1,flags,mHash[k]));
		mHash[k] = s;
	}
	
	static const int HASH_SIZE = 256;
	static const int NOT_FOUND = 0xffffffff;
	NxU32										mHash[HASH_SIZE];
	NxdSimpleArray<NxdPairFlagHashEntry <T> >	mEntries;
};


#endif
