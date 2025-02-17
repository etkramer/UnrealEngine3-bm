/*=============================================================================
	GPUSkinPublicDefs.h: Public definitions for GPU skinning.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __GPUSKINPUBLICDEFS_H__
#define __GPUSKINPUBLICDEFS_H__

/** Max number of bones that can be skinned on the GPU in a single draw call. */
#define MAX_GPUSKIN_BONES 75
/** Max number of bone influences that a single skinned vert can have. */
#define MAX_INFLUENCES 4
/** The index of the rigid influence in the BYTE[4] of influences for a skinned vertex, accounting for byte-swapping. */
#if __INTEL_BYTE_ORDER__
	#define RIGID_INFLUENCE_INDEX 0
#else
	#define RIGID_INFLUENCE_INDEX 3
#endif

#endif // __GPUSKINPUBLICDEFS_H__
