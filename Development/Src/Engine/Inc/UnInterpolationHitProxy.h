/*=============================================================================
	UnInterpolationHitProxy.h
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_INTERPOLATIONHITPROXY
#define _INC_INTERPOLATIONHITPROXY

/** Input interface hit proxy */
struct HInterpEdInputInterface : public HHitProxy
{
	FInterpEdInputInterface* ClickedObject;
	FInterpEdInputData InputData;

	DECLARE_HIT_PROXY(HInterpEdInputInterface,HHitProxy);
	HInterpEdInputInterface(FInterpEdInputInterface* InObject, const FInterpEdInputData &InData): HHitProxy(HPP_UI), ClickedObject(InObject), InputData(InData) {}

	/** @return Returns a mouse cursor from the input interface. */
	virtual EMouseCursor GetMouseCursor()
	{
		return ClickedObject->GetMouseCursor(InputData);
	}
};

struct HInterpTrackKeypointProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpTrackKeypointProxy,HHitProxy);

	class UInterpGroup*		Group;
	INT						TrackIndex;
	INT						KeyIndex;

	HInterpTrackKeypointProxy(class UInterpGroup* InGroup, INT InTrackIndex, INT InKeyIndex):
		HHitProxy(HPP_UI),
		Group(InGroup),
		TrackIndex(InTrackIndex),
		KeyIndex(InKeyIndex)
	{}

	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}
};

struct HInterpTrackKeyHandleProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(HInterpTrackKeyHandleProxy,HHitProxy);

	class UInterpGroup*		Group;
	INT						TrackIndex;
	INT						KeyIndex;
	UBOOL					bArriving;

	HInterpTrackKeyHandleProxy(class UInterpGroup* InGroup, INT InTrackIndex, INT InKeyIndex, UBOOL bInArriving):
		HHitProxy(HPP_UI),
		Group(InGroup),
		TrackIndex(InTrackIndex),
		KeyIndex(InKeyIndex),
		bArriving(bInArriving)
	{}

	virtual EMouseCursor GetMouseCursor()
	{
		return MC_Cross;
	}
};



#endif
