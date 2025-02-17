
/*
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class CoverLink_Dynamic extends CoverLink
	native;

var() bool bSkipPathUpdate;

cpptext
{
	void FindBase()
	{
		// only update the base if it's null, to prevent clobbering attachment
		if (Base == NULL)
		{
			Super::FindBase();
		}
	}

	virtual UBOOL HasFireLinkTo( INT SlotIdx, FCoverInfo &ChkCover, UBOOL bAllowFallbackLinks = FALSE );

	virtual INT AddMyMarker(AActor *S);
	virtual void PostBeginPlay();

	UBOOL ConditionalLinkSlotMarkerTo(AScout* Scout, ACoverSlotMarker* Marker, ANavigationPoint* PtToLink);
}

/** Last known stable location, where pathing was checked, etc */
var Vector LastStableLocation;

/** Should this be added to the path network even after moving?  Will still move in the octree, but will not attempt any reach tests. */
var() bool bAddToPathNetwork;
/** Should remove paths when updating */
var() bool bClearPaths;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	if( !bSkipPathUpdate )
	{
		SetTimer( 0.1f, FALSE, nameof(CheckCoverUpdate) );
	}
}

native simulated function UpdateCoverLink();
native simulated function UpdateCoverSlot(INT SlotIndex, bool bUpdateOctree=false,optional scout InScout);

function OnUpdateDynamicCover( SeqAct_UpdateDynamicCover Action )
{
	UpdateCoverLink();
}

final simulated function CheckCoverUpdate()
{
	if( !bSkipPathUpdate && !bDisabled )
	{
		if ( VSize(LastStableLocation - Location) > InvalidateDistance )
		{
			UpdateCoverLink();
		}
		// set slightly random timer to offset multiple links all updating at once
		SetTimer( 1.f + RandRange(0.1f,0.3f), FALSE, nameof(CheckCoverUpdate) );
	}
}

function OnToggle(SeqAct_Toggle inAction)
{
	Super.OnToggle(inAction);
	if (!bDisabled)
	{
		SetTimer(0.1f,FALSE,nameof(CheckCoverUpdate));
	}
}

/*
//debug
`if (`notdefined(FINAL_RELEASE))
simulated function Tick( float DeltaTime )
{
	local int SlotIdx, FireIdx, ActionIdx;
	local Vector Start, End;
	local CoverLink Targ;

	super.Tick( DeltaTime );

	if( bDebug )
	{
		FlushPersistentDebugLines();
		for( SlotIdx = 0; SlotIdx < Slots.Length; SlotIdx++ )
		{
			if( Slots[SlotIdx].SlotOwner != None )
			{
				for( FireIdx = 0; FireIdx < Slots[SlotIdx].FireLinks.Length; FireIdx++ )
				{
					for( ActionIdx = 0; ActionIdx < Slots[SlotIdx].FireLinks[FireIdx].CoverActions.Length; ActionIdx++ )
					{
						Start = GetSlotViewPoint( SlotIdx, Slots[SlotIdx].FireLinks[FireIdx].CoverType, Slots[SlotIdx].FireLinks[FireIdx].CoverActions[ActionIdx] );
						Targ  = CoverLink(Slots[SlotIdx].FireLinks[FireIdx].TargetLink.Nav);
						End   = Targ.GetSlotLocation(Slots[SlotIdx].FireLinks[FireIdx].TargetSlotIdx);

						DrawDebugLine( Start, End, 0, 255, 0, TRUE );
					}
				}

				for( FireIdx = 0; FireIdx < Slots[SlotIdx].RejectedFireLinks.Length; FireIdx++ )
				{
					Start = GetSlotLocation( SlotIdx );
					End = Slots[SlotIdx].RejectedFireLinks[FireIdx].Link.GetSlotLocation(Slots[SlotIdx].RejectedFireLinks[FireIdx].SlotIdx);
					DrawDebugLine( Start, End, 255, 0, 0, TRUE );
				}
			}
		}
	}
}
`endif
*/

// called after a slot's index is changed to notify claimants, as well as fix indices
simulated event SlotIndexUpdated(int NewIndex,int OldIndex)
{
	local GearPawn GP;
	local int Delta;

	Slots[NewIndex].SlotMarker.OwningSlot.SlotIdx = NewIndex;

	// notify the owner if any, of the index change
	if(Slots[NewIndex].SlotOwner != None)
	{
		GP = GearPawn(Slots[NewIndex].SlotOwner);
		if(GP != none)
		{
			//`log(GetFuncName()@GP@"CurrentSlotIdx:"@GP.CurrentSlotIdx@"newIndex:"@newIndex@"oldIndex"@OldIndex@"delta:"@abs(newIndex - GP.CurrentSlotIdx)@"SlotMarkerdata:"@Slots[NewIndex].SlotMarker@Slots[NewIndex].SlotMarker.OwningSlot.SlotIdx);
			// if the index just moved a lot, bail
			if(abs(newIndex - OldIndex) > 1)
			{
				NotifySlotOwnerCoverDisabled( NewIndex );
			}
			else if(GP.CurrentSlotIdx != -1)
			{
				Delta = NewIndex - OldIndex;
				//`log("Setting cover info on :"@GP@"NewIdx:"@NewIndex@"CurrentIdx:"@GP.CurrentSlotIdx@"Delta:"@Delta@WorldInfo.TimeSeconds);
				GP.SetCoverInfo(self,NewIndex,Clamp(GP.LeftSlotIdx+Delta,0,Slots.length-1),Clamp(GP.RightSlotIdx+Delta,0,Slots.length-1),GP.CurrentSlotPct,false);
			}
		}
	}
}

simulated event string GetDebugAbbrev()
{
	return "DynCL";
}

defaultproperties
{
	bStatic=FALSE
	bMovable=TRUE
//	bNoAutoConnect=TRUE
	bDynamicCover=TRUE
	bClearPaths=TRUE
//	bSkipPathUpdate=TRUE

	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true

//	bDebug=TRUE
}
