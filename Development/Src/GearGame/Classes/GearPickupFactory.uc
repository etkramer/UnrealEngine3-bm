
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPickupFactory extends PickupFactory
	abstract
	native
	nativereplication
	dependson(GearTypes)
	hidecategories(Display,Collision);


/** The light environment for this Pickup so we can see the item in our statically lit levels **/
var LightEnvironmentComponent MyLightEnvironment;

var const ActionInfo PickupAction;

var CanvasIcon NoSymbolIcon;

var bool bDrawNoSymbolIfNeeded;

var MaterialInstanceConstant MIC_WeaponSkin;

var transient Actor ClaimedBy;

cpptext
{
	virtual void PostEditChange(UProperty *PropertyThatChanged);
	virtual void PostLoad();
	virtual INT* GetOptimizedRepList(BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
}

replication
{
	if( bNetDirty && Role == ROLE_Authority )
		ClaimedBy;
}

function PostBeginPlay()
{
	Super.PostBeginPlay();

	//@HACK: hack around LD doing evil changing of bHidden through editactor window. Disable that in the editor!!!
	if (bHidden)
	{
		SetForcedInitialReplicatedProperty(Property'Engine.Actor.bHidden', true);
	}
}

function ClaimPickUp(Actor Recipient)
{
	ClaimedBy = Recipient;
	bForceNetUpdate = TRUE;
}

event Reset()
{
	Super.Reset();

	// Reset claimedby
	ClaimedBy = None;
	bForceNetUpdate = TRUE;
}

function bool ShouldSaveForCheckpoint()
{
	return TRUE;
}

function CreateCheckpointRecord(out NavigationPoint.CheckpointRecord Record)
{
	// check to see if it is active based on the current state
	Record.bDisabled = IsInState('Disabled');

	//@HACK: some LD did a bad thing and set bHidden with 'editactor' so we have to record it
	Record.bBlocked = bHidden;
}

function ApplyCheckpointRecord(const out NavigationPoint.CheckpointRecord Record)
{
	if (Record.bDisabled)
	{
		ShutDown();
	}
	else
	{
		Reset();
		//@HACK: some LD did a bad thing and set bHidden with 'editactor' so we have to record it
		SetHidden(Record.bBlocked);
	}
}

simulated protected function bool CanPickUpWhileHoldingHeavyWeapon(name Interaction)
{
	return (Interaction == 'TakeAmmo' || Interaction == 'Grab');
}

auto state Pickup
{
	event BeginState(name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);
		ClaimedBy = None;
		bForceNetUpdate = TRUE;
	}

	/** return true if Pawn can grab this pickup */
	simulated function bool CanBePickedUpBy( Pawn P )
	{
		local GearPawn MyGearPawn;
		local Name Interaction;

		if( ValidTouch(P) && (ClaimedBy == None || ClaimedBy.bDeleteMe || ClaimedBy == P) )
		{
			Interaction = FindInteractionWith(P,false);
			MyGearPawn = GearPawn(P);
			if ( Interaction != '' &&
				MyGearPawn != None &&
				!MyGearPawn.IsAKidnapper() &&
				(MyGearPawn.MyGearWeapon == None || !MyGearPawn.MyGearWeapon.bNoInteractionWhileEquipped || CanPickUpWhileHoldingHeavyWeapon(Interaction) ) )
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	simulated function bool GetPickupAction(out ActionInfo out_PickupAction, Pawn P)
	{
		// don't show any icon if touching through wall
		return (ValidTouch(P) && Global.GetPickupAction(out_PickupAction, P));
	}

	/**
	* Validate touch (if valid return true to let other pick me up and trigger event).
	* Overridden to remove call to WorldInfo.Game.PickupQuery(Other, InventoryType, self) since we need this to be client side
	*/
	simulated function bool ValidTouch( Pawn Other )
	{
		// make sure its a live player
		if( Other == None ||
			!Other.bCanPickupInventory ||
			(Other.DrivenVehicle == None && Other.Controller == None) )
		{
			return FALSE;
		}

		// make sure not touching through wall
		if( !FastTrace(Other.Location, Location + vect(0,0,16)) &&
			!FastTrace(Other.GetPawnViewLocation(), Location + vect(0,0,16)) )
		{
			return FALSE;
		}

		return TRUE;
	}

	/** disable normal touching. we require input from the player to pick it up */
	event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
	{
	}
}


/**
 * returns the display name of the item to be picked up
 */
simulated function String GetDisplayName()
{
	return GetHumanReadableName();
}

/**
 * returns true if Pawn can grab this pickup
 */
simulated function bool CanBePickedUpBy(Pawn P)
{
	return FALSE;
}

/**
 * See if we can do something with this weapon pickup.
 * bExcludeChecks - TRUE = could you have?
 *                - FALSE = you actually can
 */
simulated function Name FindInteractionWith(Pawn P, bool bExcludeChecks)
{
	// by default, pick up
	return 'PickUp';
}

/**
 * Returns TRUE if Pawn P can pick up ammo of this AmmoType
 * bExcludeChecks - TRUE = could you have?
 *                - FALSE = you actually can
 */
simulated function bool CanPickUpAmmoType(Pawn P, class<GearAmmoType> AmmoTypeClass, bool bExcludeChecks)
{
	local GearInventoryManager InvManager;

	InvManager = GearInventoryManager(P.InvManager);
	return InvManager.CanPickUpAmmoType(AmmoTypeClass, bExcludeChecks);
}


/**
 * Finds out if player is carrying weapon, and adds ammunition if so.
 * Returns TRUE if sucessful
 */
function bool PickUpAmmoFromWeaponClass(Pawn P, class<GearWeapon> WeaponClass)
{
	local GearInventoryManager InvManager;

	if (ClassIsChildOf(WeaponClass, class'GearWeap_GrenadeBase'))
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpGrenades, P, None);
	}
	else
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpAmmo, P, None);
	}

	InvManager	= GearInventoryManager(P.InvManager);
	return InvManager.AddAmmoFromWeaponClass(WeaponClass, 0);
}


/**
 * Queried by clients to get the action info to display on the HUD.
 */
simulated function bool GetPickupAction(out ActionInfo out_PickupAction, Pawn P)
{
	local Name				Interaction;

	Interaction = FindInteractionWith(P,true);

	// interactable
	if( !bPickupHidden && (Interaction != '') )
	{
		out_PickupAction = PickupAction;
		out_PickupAction.ActionName = Name;

		// replace the pickup icon with swap
		if( Interaction == 'SwapWithCurrent' || Interaction == 'SwapCarried' )
		{
			out_PickupAction.ActionIconDatas[1].ActionIcons[0].U = 242;
		}

		if ( InventoryType != None )
		{
			if ( ClassIsChildOf(InventoryType, class'GearWeap_GrenadeBase') || (InventoryType.Name == 'GearWeap_HOD') )
			{
				// replace the ammo icon with weapon icon
				out_PickupAction.ActionIconDatas[2].ActionIcons[0] = class<GearWeapon>(InventoryType).default.WeaponIcon;
			}
		}

		// if it is something that can be interacted with but can't be picked up, apply the NO symbol
		if ( !CanBePickedUpBy(P) )
		{
			// override so we don't show the NO symbol.
			if ( !bDrawNoSymbolIfNeeded )
			{
				return FALSE;
			}
			out_PickupAction.ActionIconDatas[0].ActionIcons[0] = NoSymbolIcon;
		}

		return TRUE;
	}
	// not interactable
	else
	{
		return FALSE;
	}
}

simulated function SetPickupMesh()
{
	local SkeletalMeshComponent	SMC;

	Super.SetPickupMesh();

	SMC = SkeletalMeshComponent(PickupMesh);
	if( SMC != None )
	{
		// Force Skeletal Meshes into ref pose. They're not animated.
		SMC.SetAnimTreeTemplate(None);
		SMC.bForceRefpose = 1;
		SMC.bUpdateSkelWhenNotRendered=FALSE;

		SMC.SetLightEnvironment( MyLightEnvironment );

		CreateAndSetMICWeaponSkin( SMC );
	}
}


/** This will create and set the various params on the Weapon's MIC **/
simulated function CreateAndSetMICWeaponSkin( MeshComponent MeshComp )
{
	// this component is destroyed when picked up so we don't need to worry about changing
	// the emissive color back to the default
	MIC_WeaponSkin = new(outer) class'MaterialInstanceConstant';
	MIC_WeaponSkin.SetParent( MeshComp.GetMaterial( 0 ) );
	MeshComp.SetMaterial( 0, MIC_WeaponSkin );

	// setting the emissive higher should make the weapons "stand out" on the ground a bit more
	// this may still not be enough
	MIC_WeaponSkin.SetScalarParameterValue( 'Weap_SpecMult', 5.0f );
	MIC_WeaponSkin.SetScalarParameterValue( 'Weap_DroppedGlow', 1.0f );
	MIC_WeaponSkin.SetScalarParameterValue( 'Weap_DropGlowAlpha', 1.0f );

	MIC_WeaponSkin.SetVectorParameterValue( 'Weap_EmisColor', GetEmissiveColorParam() );
}

/** This is a function to allow subclasses to return an emissive LinearColor. (e.g. some weapons need to be colored and others don't) **/
simulated function LinearColor GetEmissiveColorParam()
{
	local LinearColor LC;
	return LC;
}



defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	bCollideActors=TRUE
	bBlockActors=false
	bNoDelete=TRUE

	bDrawNoSymbolIfNeeded=TRUE

	// define here as lot of sub classes which have moving parts will utilize this
 	Begin Object Class=DynamicLightEnvironmentComponent Name=PickupLightEnvironment
 	    bDynamic=FALSE
 	    bCastShadows=FALSE
		AmbientGlow=(R=0.3f,G=0.3f,B=0.3f,A=1.0f)
		TickGroup=TG_DuringAsyncWork
 	End Object
  	MyLightEnvironment=PickupLightEnvironment
  	Components.Add(PickupLightEnvironment)


	Begin Object NAME=CollisionCylinder
		CollisionRadius=+00060.000000
		CollisionHeight=+00020.000000
		CollideActors=TRUE
		BlockZeroExtent=FALSE
	End Object

	PickupAction={(
			 ActionName=PickupFactoryIcon,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=360,V=0,UL=58,VL=55))),
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=156,V=233,UL=34,VL=56)))	),
			)}

	NoSymbolIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=411,V=342,UL=49,VL=49)
}
