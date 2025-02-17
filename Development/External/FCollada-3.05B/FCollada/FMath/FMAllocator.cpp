/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FMAllocator.h"

namespace fm
{
	/* @ UE3 fixed static initialization order crash

	// default to something: static initialization!
	AllocateFunc af = malloc;
	FreeFunc ff = free;
	
	void SetAllocationFunctions(AllocateFunc a, FreeFunc f)
	{
		af = a;
		ff = f;
	}
	*/

	// These two are simple enough, but have the advantage of
	// always allocating/releasing memory from the same heap.
	void* Allocate(size_t byteCount)
	{
		return malloc(byteCount);
// @ UE3		return (*af)(byteCount);
	}

	void Release(void* buffer)
	{
		free(buffer);
// @ UE3		(*ff)(buffer);
	}
};

