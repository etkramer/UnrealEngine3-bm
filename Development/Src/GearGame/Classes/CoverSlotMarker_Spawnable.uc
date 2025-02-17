/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class CoverSlotMarker_Spawnable extends CoverSlotMarker_Dynamic;


var repnotify int OctreeUpdateIndex;

struct ReplicatedCoverSlot
{
	var bool bLeanleft, bLeanRight;
	var bool bCanPopUp, bCanMantle;
	var ECoverType CoverType;
	var vector LocationOffset;
	var rotator RotationOffset;
	var bool bCanCoverSlip_Left, bCanCoverSlip_Right;
	var bool bCanSwatTurn_Left, bCanSwatTurn_Right;
	var int Idx;
	var CoverLink Link;
	var repretry CoverReference MantleTarget;
};

var repnotify repretry ReplicatedCoverSlot ReplicatedSlot;

var vector LastOctreeUpdateLocation;
var() float OctreeUpdateThreshold;

replication
{
	if (Role == ROLE_Authority)
		ReplicatedSlot,OctreeUpdateIndex;
}

simulated function PostBeginPlay()
{
	RefreshOctreePosition();
	LastOctreeUpdateLocation = Location;
	Super.PostBeginPlay();
}

function OctreeUpdate()
{
	if(VSizeSq(LastOctreeUpdateLocation - Location) > OctreeUpdateThreshold * OctreeUpdateThreshold)
	{
		LastOctreeUpdateLocation = Location;
		RefreshOctreePosition();
		OctreeUpdateIndex++;
	}
}

function SetCoverInfo(CoverLink Link, int Idx, CoverSlot Slot)
{
	// fill in the info
	ReplicatedSlot.bCanMantle = Slot.bCanMantle;
	ReplicatedSlot.bLeanRight = Slot.bLeanRight;
	ReplicatedSlot.bCanPopUp = Slot.bCanPopUp;
	ReplicatedSlot.bCanMantle = Slot.bCanMantle;
	ReplicatedSlot.bCanCoverSlip_Left = Slot.bCanCoverSlip_Left;
	ReplicatedSlot.bCanCoverSlip_Right = Slot.bCanCoverSlip_Right;
	ReplicatedSlot.bCanSwatTurn_Left = Slot.bCanSwatTurn_Left;
	ReplicatedSlot.bCanSwatTurn_Right = Slot.bCanSwatTurn_Right;
	ReplicatedSlot.CoverType = Slot.CoverType;
	ReplicatedSlot.LocationOffset = Slot.LocationOffset;
	ReplicatedSlot.RotationOffset = Slot.RotationOffset;
	ReplicatedSlot.Idx = Idx;
	ReplicatedSlot.Link = Link;
	ReplicatedSlot.MantleTarget = Slot.MantleTarget;
	// force a replicaiton
	ForceNetRelevant();
}

simulated event ReplicatedEvent(Name VarName)
{
	local CoverLink Link;

	if (VarName == nameof(ReplicatedSlot))
	{
		// apply the info to the cover slot
		if (ReplicatedSlot.Link != None)
		{
			Link = ReplicatedSlot.Link;

			if(Link.Slots.Length-1 < ReplicatedSlot.Idx )
			{
				Link.Slots.Length = ReplicatedSlot.Idx+1;
			}
			// apply all of the slot information
			Link.Slots[ReplicatedSlot.Idx].bCanMantle = ReplicatedSlot.bCanMantle;
			Link.Slots[ReplicatedSlot.Idx].bLeanRight = ReplicatedSlot.bLeanRight;
			Link.Slots[ReplicatedSlot.Idx].bCanPopUp = ReplicatedSlot.bCanPopUp;
			Link.Slots[ReplicatedSlot.Idx].bCanMantle = ReplicatedSlot.bCanMantle;
			Link.Slots[ReplicatedSlot.Idx].bCanCoverSlip_Left = ReplicatedSlot.bCanCoverSlip_Left;
			Link.Slots[ReplicatedSlot.Idx].bCanCoverSlip_Right = ReplicatedSlot.bCanCoverSlip_Right;
			Link.Slots[ReplicatedSlot.Idx].bCanSwatTurn_Left = ReplicatedSlot.bCanSwatTurn_Left;
			Link.Slots[ReplicatedSlot.Idx].bCanSwatTurn_Right = ReplicatedSlot.bCanSwatTurn_Right;
			Link.Slots[ReplicatedSlot.Idx].CoverType = ReplicatedSlot.CoverType;
			Link.Slots[ReplicatedSlot.Idx].LocationOffset = ReplicatedSlot.LocationOffset;
			Link.Slots[ReplicatedSlot.Idx].RotationOffset = ReplicatedSlot.RotationOffset;
			Link.Slots[ReplicatedSlot.Idx].MantleTarget = ReplicatedSlot.MantleTarget;
			// make sure the marker is correct
			Link.Slots[ReplicatedSlot.Idx].SlotMarker = self;
			// and fill in the owning slot info as well for cover searches
			OwningSlot.Link = Link;
			OwningSlot.SlotIdx = ReplicatedSlot.Idx;
		}
	}
	else if(VarName == 'OctreeUpdateIndex')
	{
		RefreshOctreePosition();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

defaultproperties
{
	bNoDelete=FALSE
	bAlwaysRelevant=TRUE
	RemoteRole=ROLE_SimulatedProxy
	bUpdateSimulatedPosition=true
	OctreeUpdateThreshold=100.f
}
