/*=============================================================================
	FMallocXenon.h: Xenon allocators
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _F_MALLOC_XENON_H_
#define _F_MALLOC_XENON_H_

#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
#pragma pack(push,8)
#include <xbdm.h>
#pragma pack(pop)
#endif

// toggle the allocator desired
//#define USE_FMALLOC_DL	1
#define USE_FMALLOC_POOLED	1

#ifndef USE_FMALLOC_DL
#define USE_FMALLOC_DL	0		// Doug Lea allocator
#endif

#ifndef USE_FMALLOC_POOLED
#define USE_FMALLOC_POOLED	0	// Pooled allocator
#endif

#if USE_FMALLOC_DL
#include "FMallocXenonPooled.h"
#include "FMallocXenonDL.h"
extern FMallocXenonDL Malloc;
#else if USE_FMALLOC_POOLED 
#include "FMallocXenonPooled.h"
extern FMallocXenonPooled Malloc;
#endif



#endif //_F_MALLOC_XENON_H_


