/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearDroppedPickup_Shield extends GearDroppedPickup
	native(Weapon);

cpptext
{
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
};

var repnotify bool bDeployed;
var repnotify bool bKicked;
var CoverLink_Spawnable DeployedLink_Rear, DeployedLink_Forward;
var SkeletalMeshComponent ShieldMeshComp;


replication
{
	if (Role == ROLE_Authority)
		bDeployed, bKicked, DeployedLink_Rear, DeployedLink_Forward;
}

simulated event ReplicatedEvent(Name VarName)
{
	if (VarName == 'InventoryClass' && bDeployed)
	{
		// intentionally ignored
	}
	else if (VarName == 'bDeployed')
	{
		if (bDeployed)
		{
			SetPickupMesh(None);
			CreateDeployedMesh();
		}
		else
		{
			CleanUpDeployment();
		}
	}
	else if (VarName == 'bKicked')
	{
		if (bKicked)
		{
			if (ShieldMeshComp == None)
			{
				CreateDeployedMesh();
			}
			// simulate the kick over, setting up the mesh component, etc
			KickOver();
		}
		else
		{
			// otherwise hide the shield mesh component
			DetachComponent(ShieldMeshComp);
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function PickedUpBy(Pawn P)
{
	local GearPawn GP;

	super.PickedUpBy(P);

	bKicked = FALSE;

	CleanUpDeployment();

	GP = GearPawn(P);
	if (GP != None)
	{
		GP.EquipShield(GearInventoryManager(GP.InvManager).Shield);
	}
	// check to see if we are an MP game
	if( GearGRI(WorldInfo.GRI).IsMultiPlayerGame() == TRUE )
	{
		GearGame(WorldInfo.Game).GearBroadcastWeaponTakenMessage( P.Controller, class'GDT_ShieldBash' );
	}
}

simulated function Name FindInteractionWith(Pawn P, bool bExcludeChecks)
{
	local GearPawn GP;
	GP = GearPawn(P);
	if (GP != None && GP.CoverType != CT_None)
	{
		return 'None';
	}
	else if (bDeployed && CanKickOver(GearPawn(P)))
	{
		return 'KickShield';
	}
	else if (!bDeployed || CanPickupDeployed(GearPawn(P)))
	{
		return 'PickupNonWeapon';
	}
	return 'None';
}

simulated function bool GetPickupAction(out ActionInfo out_PickupAction, Pawn P)
{
	local Name Interaction;

	Interaction = FindInteractionWith(P, TRUE);
	if (Interaction == 'PickupNonWeapon')
	{
		out_PickupAction = PickupAction;
		out_PickupAction.ActionName = Name;

		out_PickupAction.ActionIconDatas[2].ActionIcons[0] = class'GearShield'.default.PickupIcon;

		// if it is something that can be interacted with but can't be picked up, apply the NO symbol
		if (!CanBePickedUpBy(P))
		{
			out_PickupAction.ActionIconDatas[0].ActionIcons[0] = NoSymbolIcon;
		}
		return TRUE;
	}
	else if (Interaction == 'KickShield')
	{
		out_PickupAction = class'SeqEvt_DoorOpen'.default.InteractAction;
		return TRUE;
	}

	return FALSE;
}

/**
 * creates the component for a deployed shield
 *
 */
simulated final function CreateDeployedMesh()
{
	local class<GearShield> ShieldClass;
	local AnimNodeSequence AnimNode;

	ShieldClass = class<GearShield>(InventoryClass);
	ShieldMeshComp = new(self) ShieldClass.default.DeployedMesh.Class(ShieldClass.default.DeployedMesh);
	ShieldMeshComp.SetRotation(rot(0,-16384,0));
	ShieldMeshComp.SetLightEnvironment(MyLightEnvironment);

	CreateAndSetMICWeaponSkin( ShieldMeshComp );

	AttachComponent(ShieldMeshComp);
	AnimNode = AnimNodeSequence(ShieldMeshComp.Animations);
	AnimNode.SetAnim(ShieldClass.default.ExpandAnimName);
	AnimNode.SetPosition(AnimNode.AnimSeq.SequenceLength, false);

	// make the collision slightly bigger for catching rr->kick
	SetCollisionSize(80.0, 20.0);

	SetCollision(true, true);
	bProjTarget = TRUE;
}

function Deploy(GearPawn GP)
{
	local vector DeployLoc, HitLoc, HitNormal, FoundLocation, CheckLocation, Extent;
	local Actor HitActor;

	if (GP != None && GP.IsCarryingShield())
	{
		bDeployed = TRUE;
		DeployLoc = Location;
		// drop down to the ground
		HitActor = Trace(HitLoc,HitNormal,DeployLoc - vect(0,0,256.f),DeployLoc,TRUE,vect(1.f,1.f,1.f));
		if (HitActor != None && !IsZero(HitLoc))
		{
			// set slightly above the hit so that physics will still collide with the world
			DeployLoc = HitLoc + vect(0,0,8);
		}
		// place the shield in front of the pawn
		SetLocation(DeployLoc);
		SetRotation(GP.Rotation);
		// add a mesh to block players
		CreateDeployedMesh();
		// create the two cover links for the shield
		// we always create the link where the Pawn is standing, since obviously that works
		DeployedLink_Rear = Spawn(class'CoverLink_Spawnable',,, Location - vector(Rotation) * 24.f + vect(0,0,32), Rotation,, true);
		// check if a Pawn can fit on the other side
		CheckLocation = Location + vector(Rotation) * 112.f; // needs to be this large due to mantle distance
		CheckLocation.Z = GP.Location.Z;
		FoundLocation = CheckLocation;
		Extent = GP.GetCollisionExtent();
		Extent.X *= 1.5;
		Extent.Y = Extent.X;
		if (FindSpot(Extent, FoundLocation) && VSize(FoundLocation - CheckLocation) < 0.1)
		{
			DeployedLink_Forward = Spawn(class'CoverLink_Spawnable',,, Location + vector(Rotation) * 54.f + vect(0,0,32), Rotation - rot(0,32768,0),, true);
		}
		SetupCoverLink(DeployedLink_Rear, DeployedLink_Forward);
		if (DeployedLink_Forward != None)
		{
			SetupCoverLink(DeployedLink_Forward, DeployedLink_Rear);
		}
		GearGame(WorldInfo.Game).DeployedShields.AddItem(self);
		bForceNetUpdate = true;
	}
}

final function SetupCoverLink(CoverLink_Spawnable NewLink, Actor MantleTarget)
{
	local CoverSlotMarker_Spawnable NewMarker;
	local CoverSlot Slot;

	// create and add a slot
	Slot.ForceCoverType = CT_MidLevel;
	Slot.CoverType = CT_MidLevel;
	Slot.bLeanLeft = TRUE;
	Slot.bLeanRight = TRUE;
	Slot.bCanPopUp = TRUE;
	Slot.bCanMantle = (MantleTarget != None);
	Slot.bCanCoverSlip_Left = !Slot.bCanMantle;
	Slot.bCanCoverSlip_Right = !Slot.bCanMantle;
	Slot.bCanSwatTurn_Left = FALSE;
	Slot.bCanSwatTurn_Right = FALSE;
	Slot.bEnabled = TRUE;
	Slot.LocationOffset = TransformVectorByRotation(NewLink.Rotation, vect(12.f,0,0), true);
	Slot.RotationOffset = rot(0,0,0);
	Slot.MantleTarget.Actor = MantleTarget;
	NewLink.Slots[0] = Slot;
	// setup a marker
	NewMarker = Spawn(class'CoverSlotMarker_Spawnable',,, NewLink.GetSlotLocation(0), NewLink.GetSlotRotation(0),, true);
	NewLink.Slots[0].SlotMarker = NewMarker;
	NewMarker.OwningSlot.Link = NewLink;
	NewMarker.OwningSlot.SlotIdx = 0;
	NewMarker.SetCoverInfo(NewLink,0,Slot);
	// add the link the navigation network so we can take cover on it later
	NewLink.bAddToPathNetwork = TRUE;
	NewLink.UpdateCoverLink();
	NewLink.UpdateCoverSlot(0, true);
}

/** Can the pawn kick this shield over?  Return TRUE if deployed and the Pawn is in front of the shield. */
simulated function bool CanKickOver(GearPawn GP)
{
	return bDeployed && !bKicked && !GP.IsCarryingAHeavyWeapon() && (Normal(GP.Location - Location) dot vector(Rotation) > 0.2f) && GP.CanDoSpecialMove(SM_DoorKick) && (vector(GP.Rotation) dot Normal(Location - GP.Location) > 0.2f);
}

simulated function bool CanPickupDeployed(GearPawn GP)
{
	return bDeployed && (Normal(GP.Location - Location) dot vector(Rotation) < 0.2f) && (vector(GP.Rotation) dot Normal(Location - GP.Location) > 0.2f);
}

simulated function KickOver()
{
	local vector KickDir;

	KickDir = -vector(Rotation); // need to record here as the switch to PHYS_RigidBody will change it
	if (Role == ROLE_Authority)
	{
		bKicked = TRUE;
		SetLocation(Location + vect(0,0,1) * 32);
		SetRotation(Rotation - rot(0,-16384,0));
	}
	CollisionComponent = ShieldMeshComp;
	ShieldMeshComp.PhysicsWeight = 1.0;
	ShieldMeshComp.SetHasPhysicsAssetInstance(true);
	SetPhysics(PHYS_RigidBody);
	SetCollision(true, false);
	bBlockActors = FALSE;
	bCollideWorld = TRUE;
	ShieldMeshComp.InitRBPhys();
	ShieldMeshComp.SetBlockRigidBody(TRUE);
	ShieldMeshComp.SetRBCollidesWithChannel(RBCC_Default,TRUE);
	ShieldMeshComp.SetRBCollidesWithChannel(RBCC_BlockingVolume,TRUE);
	ShieldMeshComp.SetRBCollidesWithChannel(RBCC_Pawn, false);
	ShieldMeshComp.SetRBChannel(RBCC_GameplayPhysics);
	ShieldMeshComp.SetRBLinearVelocity(KickDir * 384.f + vect(0,0,64.f), false);
	CleanUpDeployment(TRUE);
	SetTimer( 1.f,FALSE,nameof(TurnOffPawnCollision) );
}

simulated function TurnOffPawnCollision()
{
	if (ShieldMeshComp != None)
	{
		ShieldMeshComp.SetRBCollidesWithChannel(RBCC_Pawn,FALSE);
	}
}

simulated function CleanUpDeployment(optional bool bFromKick)
{
	local GearPC PC;
	local GearAI AI;
	local int Idx;
	local GearPawn GP;

	bDeployed = FALSE;
	if (Role == ROLE_Authority)
	{
		if (DeployedLink_Rear != None)
		{
			for (Idx = 0; Idx < DeployedLink_Rear.Claims.Length; Idx++)
			{
				GP = GearPawn(DeployedLink_Rear.Claims[Idx]);
				PC = GearPC(DeployedLink_Rear.Claims[Idx].Controller);
				if (PC != None)
				{
					PC.ClientBreakFromCover();
				}
				else
				{
					AI = GearAI(DeployedLink_Rear.Claims[Idx].Controller);
					if (AI != None)
					{
						AI.InvalidateCover();
					}
				}
				if (bFromKick && Role == ROLE_Authority && GP != None)
				{
					if (GP.IsCarryingAHeavyWeapon())
					{
						GP.ThrowActiveWeapon();
					}
					GP.DoStumbleFromMeleeSpecialMove();
				}
			}
			DeployedLink_Rear.Destroy();
			DeployedLink_Rear = None;
		}
		if (DeployedLink_Forward != None)
		{
			for (Idx = 0; Idx < DeployedLink_Forward.Claims.Length; Idx++)
			{
				PC = GearPC(DeployedLink_Forward.Claims[Idx].Controller);
				if (PC != None)
				{
					PC.ClientBreakFromCover();
				}
				else
				{
					AI = GearAI(DeployedLink_Forward.Claims[Idx].Controller);
					if (AI != None)
					{
						AI.InvalidateCover();
					}
				}
			}
			DeployedLink_Forward.Destroy();
			DeployedLink_Forward = None;
		}

		GearGame(WorldInfo.Game).DeployedShields.RemoveItem(self);
	}
}

event Bump(Actor Other, PrimitiveComponent OtherComp, vector HitNormal)
{
	local GearPawn P;
	local int i;
	local bool bFriendlyShield;

	P = GearPawn(Other);
	if (P != None && GearAI(P.Controller) != None)
	{
		if (DeployedLink_Forward != None)
		{
			for (i = 0; i < DeployedLink_Forward.Claims.length; i++)
			{
				if (WorldInfo.GRI.OnSameTeam(DeployedLink_Forward.Claims[i], P))
				{
					bFriendlyShield = true;
					break;
				}
			}
		}
		if (!bFriendlyShield && DeployedLink_Rear != None)
		{
			for (i = 0; i < DeployedLink_Rear.Claims.length; i++)
			{
				if (WorldInfo.GRI.OnSameTeam(DeployedLink_Rear.Claims[i], P))
				{
					bFriendlyShield = true;
					break;
				}
			}
		}
		// AI kicks over enemy shields in its way
		//@todo: should check if can take cover here! but it needs firelinks and such...
		if (!bFriendlyShield && CanKickOver(P))
		{
			SetTimer(0.85, false, nameof(KickOver));
			P.ServerDoSpecialMove(SM_DoorKick, true);
		}
	}
}

event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	Super.Touch(Other, OtherComp, HitLocation, HitNormal);

	// prevent shield stacking
	if ( !bScriptInitialized &&
		( (Other.IsA('GearDroppedPickup_Shield') && GearDroppedPickup_Shield(Other).bDeployed) ||
			(Other.IsA('GearPawn') && Other != Instigator) ) )
	{
		Destroy();
	}
}

event Destroyed()
{
	Super.Destroyed();

	CleanUpDeployment(false);
}

simulated singular event BaseChange()
{
	if (Pawn(Base) != None)
	{
		SetBase(None);
	}
}

// ignore this because it can happen when mantling over the shield and cause the shield to go poof
event EncroachedBy(Actor Other);

auto state Pickup
{
	/** return true if Pawn can grab this pickup */
	simulated function bool CanBePickedUpBy(Pawn P)
	{
		local GearPawn MyGearPawn;

		MyGearPawn = GearPawn(P);
		return (MyGearPawn != None && MyGearPawn.EquippedShield == None && Super.CanBePickedUpBy(P));
	}
}

defaultproperties
{
}
