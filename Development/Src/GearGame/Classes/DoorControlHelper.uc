/**
 * Helper class for SeqAct_DoorControl
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class DoorControlHelper extends GearControlHelper
	native(Sequence);

cpptext
{
	virtual void ClearCrossLevelReferences();
	virtual void GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel);
}
	
/**
 *	When SHUT input triggered BLOCK BlockWhenShut
 */
var() editconst const array<CrossLevelReachSpec> BlockedWhenShut;

final event AdjustCover()
{
	local SeqAct_DoorControl	DC;
	local int					LinkIdx, SlotIdx;
	local CoverReplicator		CoverReplicator;
	local array<int>			Slots;
	local CoverLink				Link;

	DC = SeqAct_DoorControl(SeqObj);
	if (DC!= None)
	{
		if( DC.bAutoAdjustCover )
		{
			if( !DC.bCurrentOpen )
			{
				SetBlockingVolumeCollision( TRUE, DC.BlockingVolumes );
			}

			CoverReplicator = WorldInfo.Game.GetCoverReplicator();

			// For each cover link associated with the door
			for( LinkIdx = 0; LinkIdx < DC.CoverLinks.Length; LinkIdx++ )
			{
				Slots.Length = 0;

				Link = DC.CoverLinks[LinkIdx];
				if( Link != None )
				{
					for( SlotIdx = 0; SlotIdx < Link.Slots.Length; SlotIdx++ )
					{
						Link.AutoAdjustSlot( SlotIdx, TRUE );
						Slots[Slots.Length] = SlotIdx;
					}				
				}

				if( CoverReplicator != None )
				{
					CoverReplicator.NotifyAutoAdjustSlots(Link, Slots);
				}
			}

			if( !DC.bCurrentOpen )
			{
				SetBlockingVolumeCollision( FALSE, DC.BlockingVolumes );
			}
		}
	}
}

defaultproperties
{
}
