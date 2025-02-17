/**
 * Helper class for SeqAct_PopupCoverControl
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class PopupCoverControlHelper extends GearControlHelper
	native(Sequence);

cpptext
{
	virtual void ClearCrossLevelReferences();
	virtual void GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel);
}

/**
 *	When UP input triggered BLOCK BlockedWhenUp, UNBLOCK BlockedWhenDown
 *	When DOWN input triggered BLOCK BlockWhenDown, UNBLOCK BlockWhenUp
 */
var() editconst const array<CrossLevelReachSpec> BlockedWhenUp;
var() editconst const array<CrossLevelReachSpec> BlockedWhenDown;

struct AffectedCoverLink
{
	var CoverLink Link;
	var array<int> Slots;
};

final event AdjustCover()
{
	local SeqAct_PopupCoverControl PCC;
	local array<AffectedCoverLink> LinkList;
	local AffectedCoverLink NewLink;
	local int MarkerIdx, LinkIdx;
	local CoverSlotMarker Marker;
	local CoverReplicator CoverReplicator;
	local array<int> Slots;

	PCC = SeqAct_PopupCoverControl(SeqObj);
	if (PCC != None)
	{
		// For each cover link associated with the popup cover
		for (MarkerIdx = 0; MarkerIdx < PCC.MarkerList.length; MarkerIdx++)
		{
			Marker = PCC.MarkerList[MarkerIdx];
			if (Marker != None && Marker.OwningSlot.Link != None)
			{
				Marker.SetSlotEnabled(PCC.bCurrentPoppedUp);
				LinkIdx = LinkList.Find('Link', Marker.OwningSlot.Link);
				if (LinkIdx == INDEX_NONE)
				{
					NewLink.Link = Marker.OwningSlot.Link;
					NewLink.Slots[0] = Marker.OwningSlot.SlotIdx;
					LinkList.AddItem(NewLink);
				}
				else
				{
					LinkList[LinkIdx].Slots.AddItem(Marker.OwningSlot.SlotIdx);
				}
			}
		}
		CoverReplicator = WorldInfo.Game.GetCoverReplicator();
		for (LinkIdx = 0; LinkIdx < LinkList.length; LinkIdx++)
		{
			if (CoverReplicator != None)
			{
				if (PCC.bCurrentPoppedUp)
				{
					CoverReplicator.NotifyEnabledSlots(LinkList[LinkIdx].Link, LinkList[LinkIdx].Slots);
				}
				else
				{
					CoverReplicator.NotifyDisabledSlots(LinkList[LinkIdx].Link, LinkList[LinkIdx].Slots);
				}
			}

			// If desired, do extra work to adjust cover nodes for slipping
			if(PCC.bAutoAdjustCover)
			{
				Slots.length = 0;
				for (MarkerIdx = 0; MarkerIdx < LinkList[LinkIdx].Link.Slots.length; MarkerIdx++)
				{
					LinkList[LinkIdx].Link.AutoAdjustSlot(MarkerIdx, true);
					Slots[Slots.length] = MarkerIdx;
				}
				if (CoverReplicator != None)
				{
					CoverReplicator.NotifyAutoAdjustSlots(LinkList[LinkIdx].Link, Slots);
				}
			}
		}
	}
}

defaultproperties
{
}
