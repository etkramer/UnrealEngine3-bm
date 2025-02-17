
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_PlantShield extends GSM_BasePlaySingleAnim;

var CameraAnim	CameraAnimPlantShield;

function PlayAnimation()
{
	Super.PlayAnimation();
	PawnOwner.SetTimer( 1.26f * 1.f/SpeedModifier, FALSE, nameof(self.PlantShield), self );
	PlayCameraAnim(PawnOwner, CameraAnimPlantShield);
}

function PlantShield()
{
	local GearShield Shield;
	local GearDroppedPickup_Shield DeployedShield;

	if( PawnOwner.Role == ROLE_Authority )
	{
		if( PawnOwner.IsCarryingShield() )
		{
			// find the shield and force it to deploy
			Shield = GearShield(PawnOwner.FindInventoryType(class'GearShield', TRUE));
			DeployedShield = Shield.Deploy();
			if( DeployedShield != None )
			{
				PawnOwner.DropShield(TRUE);
				if (PCOwner != None && PawnOwner.Health > 0) // can't use IsDBNO() here since it just does a specialmove check
				{
					if( PCOwner.IsLocalPlayerController() )
					{
						PCOwner.AcquireCover(GetCovPosInfoFromSlotMarker(DeployedShield.DeployedLink_Rear.Slots[0].SlotMarker));
					}
					else
					{
						// tell the client to acquire the cover
						PCOwner.ServerDictatedCover = DeployedShield.DeployedLink_Rear.Slots[0].SlotMarker;
					}
				}
			}
		}
	}
	else
	{
		//PawnOwner.DropShield(TRUE);
	}
}

function CovPosInfo GetCovPosInfoFromSlotMarker(CoverSlotMarker Marker)
{
	local CovPosInfo CovInfo;

	CovInfo.Link = Marker.OwningSlot.Link;
	CovInfo.LtSlotIdx = Marker.OwningSlot.SlotIdx;
	CovInfo.RtSlotIdx = Marker.OwningSlot.SlotIdx;
	CovInfo.LtToRtPct = 0.f;
	CovInfo.Location = Marker.Location;
	CovInfo.Normal = vector(Marker.Rotation) * -1;

	return CovInfo;
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	if( PawnOwner.IsTimerActive('PlantShield', Self) )
	{
		PawnOwner.ClearTimer('PlantShield', Self);
		PlantShield();
	}

	Super.SpecialMoveEnded(PrevMove, NextMove);
}

defaultproperties
{
	BS_Animation=(AnimName[BS_FullBody]="Shield_Deployed_Plant")
	CameraAnimPlantShield=CameraAnim'COG_MarcusFenix.Camera_Anims.ShieldPlant_Cam01'
	bCanFireWeapon=FALSE
	bShouldAbortWeaponReload=TRUE
	bLockPawnRotation=TRUE
	bDisableMovement=TRUE
}
