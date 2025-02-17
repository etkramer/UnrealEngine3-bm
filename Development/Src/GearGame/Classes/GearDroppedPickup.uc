/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearDroppedPickup extends DroppedPickup
	native
	nativereplication
	DependsOn(GearWeaponPickupFactory);

/** Who claimed this pickup */
var transient Actor	ClaimedBy;

enum EGearDroppedPickupType
{
	eGEARDROP_Generic,
	eGEARDROP_Special,
};

/** The light environment for this Pickup so we can see the item in our statically lit levels **/
var LightEnvironmentComponent MyLightEnvironment;

var ActionInfo PickupAction;

var MaterialInstanceConstant MIC_WeaponSkin;

var CanvasIcon NoSymbolIcon;

var const RigidBodyState	RBState;
var	const float				AngErrorAccumulator;

/** True if a collision should trigger audio, false otherwise. */
var transient bool			bCollisionSoundsEnabled;

/** Emissive color to send out to client**/
var repnotify byte DroppedEmissiveColorTeam;
var protected MeshComponent MyMeshComp;

cpptext
{
	// AActor interface
	virtual void physRigidBody(FLOAT DeltaTime);
	virtual INT* GetOptimizedRepList(BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel);
}


replication
{
	if ( Role == ROLE_Authority )
		RBState;

	if( bNetDirty && Role == ROLE_Authority )
		ClaimedBy, DroppedEmissiveColorTeam;
}


simulated event ReplicatedEvent(name VarName)
{
	if( VarName == 'DroppedEmissiveColorTeam' )
	{
		UpdateEmissiveColor( DroppedEmissiveColorTeam );
	}
	else
	{
		super.ReplicatedEvent(VarName);
	}
}


/** Checks the inventory item to determine when if/when to fade out */
final function CheckForWeaponFadeOut()
{
	local GearGame GG;
	local float FadeoutTime;

	if( Role == ROLE_Authority )
	{
		GG = GearGame(WorldInfo.Game);
		if( GG != None )
		{
			FadeoutTime = (GG.GetDropType(self) == eGEARDROP_Generic) ? GG.WeaponFadeTimeGeneric : GG.WeaponFadeTimeSpecial;
			SetTimer(FadeoutTime, false, nameof(FadeOut));
			SetTimer(FadeoutTime * 2.0, false, nameof(ForceFadeOut));
		}
	}
}

function FadeOut()
{
	local GearPawn P;

	// don't fade out yet if a player is nearby
	foreach WorldInfo.AllPawns(class'GearPawn', P, Location, 1024.0)
	{
		if (P.IsPlayerOwned())
		{
			SetTimer(5.0, false, nameof(FadeOut));
			return;
		}
	}
	GotoState('FadeoutWeapon');
}

/** does the fade out without the proximity check, for when the pickup has been around for a really long time */
function ForceFadeOut()
{
	GotoState('FadeoutWeapon');
}

state FadeOutWeapon extends Pickup
{
	// Can't pickup fading weapons
	simulated function bool CanBePickedUpBy( Pawn P );

	function Tick(float DeltaTime)
	{
		SetDrawScale(FMax(0.01, DrawScale - Default.DrawScale * DeltaTime));
	}

	function BeginState(Name PreviousStateName)
	{
		RotationRate.Yaw=60000;
		SetPhysics(PHYS_Rotating);
		LifeSpan = 1.0;
	}

	/** disable normal touching. we require input from the player to pick it up */
	event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
	{
	}
}


//@NOTE:   copy this Guy over to GearPickupFactory so we can have the shield glow also


/** This will create and set the various params on the Weapon's MIC **/
simulated function CreateAndSetMICWeaponSkin( MeshComponent MeshComp )
{
	// this component is destroyed when picked up so we don't need to worry about changing
	// the emissive color back to the default
	MIC_WeaponSkin = new(self) class'MaterialInstanceConstant';
	MIC_WeaponSkin.SetParent( MeshComp.GetMaterial( 0 ) );
	MeshComp.SetMaterial( 0, MIC_WeaponSkin );

	// setting the emissive higher should make the weapons "stand out" on the ground a bit more
	// this may still not be enough
	MIC_WeaponSkin.SetScalarParameterValue( 'Weap_SpecMult', 5.0f );
	MIC_WeaponSkin.SetScalarParameterValue( 'Weap_DroppedGlow', 1.0f );
	MIC_WeaponSkin.SetScalarParameterValue( 'Weap_DropGlowAlpha', 1.0f );
}

event Redrop()
{
	local SkeletalMeshComponent SkelComp;
	SetPickupMesh(CollisionComponent);
	SkelComp = SkeletalMeshComponent(CollisionComponent);
	if (SkelComp != None)
	{
		SkelComp.PhysicsAssetInstance.SetAllBodiesFixed(FALSE);
		SkelComp.WakeRigidBody();
	}
	CollisionComponent.SetRBLinearVelocity(vect(0,0,-256),TRUE);
}

/**
 * sets the pickups mesh and makes it the collision component so we can run rigid body physics on it
 * @NOTE:  if you change this make certain you update GearDroppedPickup_Shield's CreateDeployedMesh()
 */
simulated function SetPickupMesh(PrimitiveComponent NewPickupMesh)
{
	local ActorComponent Comp;
	local SkeletalMeshComponent SkelMC;

	if (NewPickupMesh != None)
	{
		if (CollisionComponent == None || CollisionComponent.Class != NewPickupMesh.Class)
		{
			Comp = new(self) NewPickupMesh.Class(NewPickupMesh);
			MyMeshComp = MeshComponent(Comp);
			AttachComponent(Comp);
			CollisionComponent = PrimitiveComponent(Comp);
			if( MIC_WeaponSkin == none )
			{
				CreateAndSetMICWeaponSkin( MeshComponent(Comp) );
			}
			SetEmissiveColor( DroppedEmissiveColorTeam );
		}
		else
		{
			Comp = CollisionComponent;
		}


		// Set up physics on the cloned component
		CollisionComponent.SetScale3D(NewPickupMesh.Scale3D);
		SkelMC = SkeletalMeshComponent(CollisionComponent);

		CollisionComponent.SetBlockRigidBody(TRUE);
		CollisionComponent.SetActorCollision(TRUE,FALSE);

		if (SkelMC != None)
		{
			// Force reference pose on dropped pickups.
			// @JF: commented this, as it causes the weapon drops to
			// not orient properly to the rigid body.

			//SkelMC.SetAnimTreeTemplate(None);
			//SkelMC.bForceRefpose = 1;

			SkelMC.bUpdateSkelWhenNotRendered=FALSE;

			SkelMC.PhysicsWeight = 1.f;
			SkelMC.SetHasPhysicsAssetInstance(TRUE);
			SetPhysics(PHYS_RigidBody);

			CollisionComponent.SetRBChannel( RBCC_Nothing );
			CollisionComponent.SetRBCollidesWithChannel( RBCC_Default, TRUE );
			CollisionComponent.SetRBCollidesWithChannel( RBCC_BlockingVolume, TRUE );

			CollisionComponent.SetNotifyRigidBodyCollision( TRUE );
			CollisionComponent.ScriptRigidBodyCollisionThreshold = 100;
			CollisionComponent.WakeRigidBody();

			SkelMC.SetLightEnvironment( MyLightEnvironment );

			SetTimer( 1.0f, TRUE, nameof(CheckForRigidBodySleepState) );
		}
	}
}


simulated function SetEmissiveColor( byte InTeamNum )
{
	DroppedEmissiveColorTeam = InTeamNum;
	UpdateEmissiveColor( DroppedEmissiveColorTeam );
	bForceNetUpdate = TRUE;
}


simulated protected function UpdateEmissiveColor( byte InTeamNum )
{
	local LinearColor LC;

	if( MIC_WeaponSkin == none && MyMeshComp != none )
	{
		CreateAndSetMICWeaponSkin( MyMeshComp );
	}

	if( MIC_WeaponSkin != none && class<GearWeapon>(InventoryClass) != none )
	{
		if( InTeamNum == 0 )
		{
			LC = class<GearWeapon>(InventoryClass).static.GetWeaponEmisColor_COG( WorldInfo, class<GearWeapon>(InventoryClass).default.WeaponID );
		}
		else
		{
			LC = class<GearWeapon>(InventoryClass).static.GetWeaponEmisColor_Locust( WorldInfo, class<GearWeapon>(InventoryClass).default.WeaponID );
		}

		//`log( "AHHHH AHHH AHHH" @ LC.R @ LC.G @ LC.B @ LC.A );
		MIC_WeaponSkin.SetVectorParameterValue( 'Weap_EmisColor', LC );
	}
}


simulated protected function bool CanPickUpWhileHoldingHeavyWeapon(name Interaction)
{
	local class<GearWeapon> WeaponClass;

	if (Interaction == 'TakeAmmo' || ClassIsChildOf(InventoryClass, class'GearWeap_GrenadeBase') )
	{
		return TRUE;
	}

	if (Interaction == 'SwapWithCurrent')
	{
		WeaponClass	= class<GearWeapon>(InventoryClass);
		if ( (WeaponClass != None) && (WeaponClass.default.WeaponType == WT_Heavy) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

auto state Pickup
{
	/** return true if Pawn can grab this pickup */
	simulated function bool CanBePickedUpBy( Pawn P )
	{
		local GearPawn MyGearPawn;
		local Name Interaction;

		MyGearPawn = GearPawn(P);
		if( ValidTouch(P) && (ClaimedBy == None || ClaimedBy == P) )
		{
			Interaction = FindInteractionWith(P,false);
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
	 * Overriden to remove call to WorldInfo.Game.PickupQuery(Other, InventoryType, self) since we need this to be client side
	 */
	simulated function bool ValidTouch( Pawn Other )
	{
		local Actor HitA;
		local vector HitLocation, HitNormal;
		// make sure its a live player
		if( Other == None ||
			!Other.bCanPickupInventory ||
			(Other.DrivenVehicle == None && Other.Controller == None) )
		{
			return FALSE;
		}

		// make sure not touching through wall
		foreach Other.TraceActors(class'Actor',HitA,HitLocation,HitNormal,Location,Other.Location,vect(1,1,1))
		{
			//`log(`showvar(HitA)@`showvar(HitA.bBlockActors));
			if (HitA != self && HitA.bBlockActors && HitA.Physics != PHYS_RigidBody)
			{
				return FALSE;
			}
			else if (HitA == self)
			{
				break;
			}
		}

		return TRUE;
	}

	/**
	* Overridden to prevent adding to navigation network.
	*/
	function BeginState(Name PreviousStateName)
	{
		//SetTimer(LifeSpan - 1, false);
	}

	/** disable normal touching. we require input from the player to pick it up */
	event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
	{
		local GearAI_TDM Bot;
		local GearPawn P;

		if (bScriptInitialized) // this is so the bots don't constantly swap when two weapons are rated equally (since they'll touch it while dropping it)
		{
			// if a bot touched me while doing something else, see if it wants to pick me up anyway
			P = GearPawn(Other);
			if (P != None && P.CanDoSpecialMove(SM_WeaponPickup))
			{
				Bot = GearAI_TDM(P.Controller);
				if (Bot != None && Bot.MoveTarget != self && Bot.AllowDetourToPickUp(InventoryClass))
				{
					class'AICmd_PickupWeapon'.static.PickupWeaponFromPickup(Bot, self);
				}
			}
		}
	}

	function CheckTouching();
}


/**
 * returns the display name of the item to be picked up
 */
simulated function String GetDisplayName()
{
	local class<GearWeapon>		WeaponClass;
	WeaponClass	= class<GearWeapon>(InventoryClass);
	if ( WeaponClass != None )
	{
		return WeaponClass.Default.ItemName;
	}

	return "";
}

/**
 * Returns TRUE if Pawn P can pick up ammo of this AmmoType
 * bExcludeChecks - TRUE = could you have?
 *                - FALSE = you actually can
 */
simulated function bool CanPickUpAmmoType(Pawn P, class<GearAmmoType> AmmoTypeClass, bool bExcludeChecks)
{
	local GearInventoryManager	InvManager;

	InvManager = GearInventoryManager(P.InvManager);
	return InvManager.CanPickUpAmmoType(AmmoTypeClass, bExcludeChecks);
}


/**
 * Finds out if player is carrying weapon, and adds ammunition if so.
 * Returns TRUE if successful
 */
function bool PickUpAmmoFromWeaponClass(Pawn P, class<GearWeapon> WeaponClass)
{
	local GearInventoryManager	InvManager;
	local GearWeapon W;

	if (ClassIsChildOf(WeaponClass, class'GearWeap_GrenadeBase'))
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpGrenades, P, None);
	}
	else
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpAmmo, P, None);
	}

	InvManager	= GearInventoryManager(P.InvManager);
	W = GearWeapon(Inventory);
	return InvManager.AddAmmoFromWeaponClass(WeaponClass, W.GetPickupAmmoAmount() );
}

/** return true if Pawn can grab this pickup */
simulated function bool CanBePickedUpBy( Pawn P )
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
	local GearInventoryManager	InvManager;
	local GearPawn.EAttachSlot	DesiredSlot;
	local class<GearWeapon>		WeaponClass;
	local GearWeapon			MyGearWeapon;

	if (P == None || GearInventoryManager(P.InvManager) == NOne)
	{
		return 'None';
	}

	// Don't allow any pickup when carrying crate
	if(GearPawn(P) != None && GearPawn(P).CarriedCrate != None)
	{
		return 'None';
	}

	InvManager	= GearInventoryManager(P.InvManager);
	WeaponClass	= class<GearWeapon>(InventoryClass);

	if (WeaponClass == None)
	{
		// picking up something that isn't a weapon
		if ( ClassIsChildOf(InventoryClass, class'GearShield') )
		{
			return 'PickupNonWeapon';
		}
	}
	// if we're already carrying that same weapon
	else if (!ClassIsChildOf(WeaponClass, class'GearWeap_HeavyBase') && InvManager.FindInventoryType(WeaponClass, TRUE) != None)
	{
		// check if we can pick up ammo from it
		if (CanPickUpAmmoType(P, WeaponClass.default.AmmoTypeClass, bExcludeChecks) )
		{
			return 'TakeAmmo';
		}
	}
	else
	{
		// See if we can find an available slot for this weapon
		DesiredSlot = InvManager.FindFreeSlotForInventoryClass(WeaponClass);

		if( DesiredSlot != EASlot_None )
		{
			//`log( "00" );
			// we found a free slot, we can pickup weapon!
			return 'PickUp';
		}

		// If we're already carrying a weapon in that slot,
		// see if we can swap our currently held weapon for this one
		if( P.CanThrowWeapon() && InvManager.CanSwapWeaponClassWithCurrent(WeaponClass) )
		{
			//`log( "22" );
			return 'SwapWithCurrent';
		}

		// If we want to pick up a pistol or a grenade, we can just grab and exchange it
		if( WeaponClass.default.WeaponType == WT_Holster ||
			WeaponClass.default.WeaponType == WT_Item )
		{
			//`log( "33" );
			return 'SwapWithAttachment';
		}

		// for heavy weapons, we pick up and hold
		MyGearWeapon = GearWeapon(P.Weapon);
		if( WeaponClass.default.WeaponType == WT_Heavy )
		{
			if( MyGearWeapon != None && MyGearWeapon.WeaponType == WT_Heavy )
			{
				return 'SwapCarried';
			}
			else
			{
				return 'PickUpAndCarry';
			}
		}
	}

	return 'None';
}

/** Internal. */
protected function TriggerPickupGUDS(GearPawn GP, Inventory NewWeap)
{
	if (GearWeap_GrenadeBase(NewWeap) != None)
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpGrenades, GP, None);
	}
	else if (GearWeap_FlameThrowerBase(NewWeap) != None)
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpFlamethrower, GP, None);
	}
	else
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpNewWeapon, GP, None);
	}
}



/**
 * Give pickup to player
 */
function GiveTo( Pawn P )
{
	local GearInventoryManager	InvManager;
	local SoundCue				AmmoPickupSound;
	local GearWeapon				InvWeapon, OtherWeapon, NewWeapon;
	local GearPawn GP;
	local Inventory NewInv;
	local name Interaction;
	local int AmmoAdded;

	GP = GearPawn(P);
	InvManager	= GearInventoryManager(P.InvManager);
	InvWeapon = GearWeapon(Inventory);

	Interaction = FindInteractionWith(P, false);
	switch (Interaction)
	{
		/** just take ammo from weapon */
		case 'TakeAmmo' :
			// Fill up ammo if we can. Otherwise leave weapon.
			if( PickUpAmmoFromWeaponClass(P, class<GearWeapon>(InventoryClass)) )
			{
				AmmoPickupSound = class<GearWeapon>(InventoryClass).default.AmmoTypeClass.default.AmmoPickupSound;
				//DupWeap.AnnouncePickup(P);
				//P.PlaySound( AmmoPickupSound );
				if(GP != none)
				{
					GP.AmmoPickupSound = AmmoPickupSound;
				}
				PickedUpBy(P);
			}
			break;

		/** throw current weapon, and pick up new one */
		case 'SwapWithCurrent' :
			if( InvWeapon != None )
			{
				//@note: intentionally not using ThrowActiveWeapon() as that would cause a SwitchToBestWeapon() we don't want
				P.TossInventory(P.Weapon);

				Inventory.AnnouncePickup(P);

				// @todo - why aren't we just giving him the inventory?
				// JF: Inventory and InventoryClass may not match, so we need to spawn in that case.
				NewWeapon = GearWeapon(SpawnCopyFor(P));

				// fix ammo count based on inventory ammocount
				if( InvWeapon != None && NewWeapon != None )
				{
					AmmoAdded = NewWeapon.CopyAmmoAmountFromWeapon( InvWeapon );
					NewWeapon.AddAmmoMessage( AmmoAdded );
				}

				TriggerPickupGUDS(GP, NewWeapon);

				// Switch to new weapon
				InvManager.SetCurrentWeapon( NewWeapon );
				PickedUpBy(P);
			}
			break;

		// Take weapon and Swap it with an attachment (ie not the currently held one
		case 'SwapWithAttachment' :
			if( InvWeapon != None )
			{
				// Toss carried weapon
				OtherWeapon = InvManager.FindWeaponOfType(InvWeapon.WeaponType);
				P.TossInventory(OtherWeapon);

				// Grab new one
				Inventory.AnnouncePickup(P);

				// @todo - why aren't we just giving him the inventory?
				NewWeapon = GearWeapon(SpawnCopyFor(P));

				// fix ammo count based on inventory ammocount
				if( InvWeapon != None && NewWeapon != None )
				{
					AmmoAdded = NewWeapon.CopyAmmoAmountFromWeapon( InvWeapon );
					NewWeapon.AddAmmoMessage( AmmoAdded );
				}
				TriggerPickupGUDS(GP, NewWeapon);
				PickedUpBy(P);

				// Switch to that new weapon only if it's not a grenade.
				if( NewWeapon != None && !NewWeapon.IsA('GearWeap_GrenadeBase') )
				{
					InvManager.SetCurrentWeapon(NewWeapon);
				}
			}
			break;

		/** Just pick up new weapon */
		case 'PickUp' :
			if( Inventory != None )
			{
				Inventory.AnnouncePickup(P);

				NewWeapon = GearWeapon(SpawnCopyFor(P));
				// fix ammo count based on inventory ammocount
				if (InvWeapon != None && NewWeapon != None )
				{
					AmmoAdded = NewWeapon.CopyAmmoAmountFromWeapon( InvWeapon );
					NewWeapon.AddAmmoMessage( AmmoAdded );
				}

				TriggerPickupGUDS(GP, NewWeapon);

				// Switch to that new weapon only if it's not a grenade.
				if( NewWeapon != None && !NewWeapon.IsA('GearWeap_GrenadeBase') )
				{
					InvManager.SetCurrentWeapon(NewWeapon);
				}
			}

			PickedUpBy(P);
			break;

		case 'SwapCarried':
			P.ThrowActiveWeapon();
			// intentional fallthrough

		case 'PickUpAndCarry':
			if (Inventory != None)
			{
				Inventory.AnnouncePickup(P);
				NewWeapon = GearWeapon(SpawnCopyFor(P));
				// fix ammo count based on inventory ammocount
				if (InvWeapon != None && NewWeapon != None )
				{
					NewWeapon.CopyAmmoAmountFromWeapon( InvWeapon );
				}

				TriggerPickupGUDS(GP, NewWeapon);

				InvManager.SetCurrentWeapon(NewWeapon);

				PickedUpBy(P);
			}
			break;

		case 'PickupNonWeapon':

			NewInv = SpawnCopyFor(P);

			// play an effort
			// @fixme, pickupshield effort?
			if (GP != None)
			{
				GP.SoundGroup.PlayEffort(GP, GearEffort_MantleEffort);
			}

			TriggerPickupGUDS(GP, NewInv);

			if (NewInv.PickupSound != None)
			{
				NewInv.PlaySound(NewInv.PickupSound);
			}

			PickedUpBy(P);
			break;
	}
}

function Inventory SpawnCopyFor( Pawn Recipient )
{
	return Recipient.InvManager.CreateInventory(InventoryClass);
}


/**
 * Overridden to prevent adding to navigation network.
 */
event Landed(vector HitNormal, actor FloorActor)
{
	bForceNetUpdate = TRUE;
	NetUpdateFrequency = 3;

	PlayCollisionSound();
}


/**
  * If this RB is asleep then we are going to turn off all collision on it.  This stops
  * the painfulness of trying to pick up a weapon and then your character kicking it out of the
  * way.
  *
 **/
simulated protected function CheckForRigidBodySleepState()
{
	local SkeletalMeshComponent SkelComp;

	//`log( "checking for CheckForRigidBodySleepState" );
	if( CollisionComponent.RigidBodyIsAwake() == FALSE )
	{
		//`log( "turning off RB collision" );
		CollisionComponent.SetNotifyRigidBodyCollision( FALSE );
		ClearTimer( 'CheckForRigidBodySleepState' );
		CollisionComponent.SetBlockRigidBody( FALSE );
		CollisionComponent.SetActorCollision( FALSE, FALSE );

		// Fix in place so nothing can move it.
		SkelComp = SkeletalMeshComponent(CollisionComponent);
		if(SkelComp != None && SkelComp.PhysicsAssetInstance != None)
		{
			SkelComp.PhysicsAssetInstance.SetAllBodiesFixed(TRUE);
			SkelComp.bUpdateKinematicBonesFromAnimation = FALSE;
		}

		// Force reference pose on dropped pickups that are not moving
		if( SkelComp != None )
		{
			SkelComp.bUpdateSkelWhenNotRendered = FALSE;
			SkelComp.SetAnimTreeTemplate(None);
			//SkelComp.bForceRefpose = 1;
		}
	}
}


/**
 * Queried by clients to get the action info to display on the HUD.
 */
simulated function bool GetPickupAction(out ActionInfo out_PickupAction, Pawn P)
{
	local Name				Interaction;
	local class<GearWeapon>	WeaponClass;

	out_PickupAction = PickupAction;
	out_PickupAction.ActionName = Name;

	// figure out what icons need to swap
	Interaction = FindInteractionWith(P,true);

	if( Interaction == 'TakeAmmo' )
	{
		WeaponClass = class<GearWeapon>(InventoryClass);
		if ( ClassIsChildOf(WeaponClass, class'GearWeap_GrenadeBase') || (WeaponClass.Name == 'GearWeap_HOD') )
		{
			// replace the ammo icon with weapon icon
			out_PickupAction.ActionIconDatas[2].ActionIcons[0] = WeaponClass.default.WeaponIcon;
		}
	}
	else
	{
		WeaponClass = class<GearWeapon>(InventoryClass);
		if (WeaponClass != None)
		{
			// replace the ammo icon with weapon icon
			out_PickupAction.ActionIconDatas[2].ActionIcons[0] = WeaponClass.default.WeaponIcon;
			// replace the pickup icon with swap
			if( Interaction == 'SwapWithCurrent' || Interaction == 'SwapCarried' )
			{
				out_PickupAction.ActionIconDatas[1].ActionIcons[0].U = 242;
			}
		}
	}

	// if it is something that can be interacted with but can't be picked up, apply the NO symbol
	if (!CanBePickedUpBy(P))
	{
		out_PickupAction.ActionIconDatas[0].ActionIcons[0] = NoSymbolIcon;
	}
	return TRUE;
}


/** @see GearWeapon.DropFrom  for the ScriptRigidBodyCollisionThreshold value we use **/
event RigidBodyCollision( PrimitiveComponent HitComponent, PrimitiveComponent OtherComponent,
						 const out CollisionImpactData RigidCollisionData, int ContactIndex)
{
	// attach to any GDOs we encounter so that the GDO can redrop us if it gets blown up
	if (OtherComponent != None && GearDestructibleObject(OtherComponent.Owner) != None)
	{
		SetBase(OtherComponent.Owner);
	}

	PlayCollisionSound();

	// no super to call here
}

function PlayCollisionSound()
{
	local SoundCue Cue;
	local float Duration;

	if (bCollisionSoundsEnabled)
	{
		if (class<GearWeapon>(InventoryClass) != None)
		{
			Cue = class<GearWeapon>(InventoryClass).default.WeaponDropSound;
		}
		else if (class<GearShield>(InventoryClass) != None)
		{
			Cue = class<GearShield>(InventoryClass).default.ShieldDropSound;
		}

		if (Cue != None)
		{
			Duration = Cue.GetCueDuration();
			SetTimer( Duration, FALSE, nameof(ReenableCollisionSounds) );
			PlaySound(Cue);
			bCollisionSoundsEnabled = FALSE;
		}
	}
}

function ReenableCollisionSounds()
{
	bCollisionSoundsEnabled = TRUE;
}

function ClaimPickUp(Actor Recipient)
{
	ClaimedBy = Recipient;
	bForceNetUpdate = TRUE;
}

defaultproperties
{
	//TickGroup=TG_DuringAsyncWork  // can't turn this on here as we get a TickGroup error when trying to update bones

	bOnlyDirtyReplication=FALSE
	bOnlyRelevantToOwner=FALSE
	bAlwaysRelevant=TRUE
	bKillDuringLevelTransition=TRUE
	Lifespan=0

	bNoEncroachCheck=TRUE // Pawns move into these actors.  So we don't care if they do not give touch events for moving

	Begin Object Class=DynamicLightEnvironmentComponent Name=DroppedPickupLightEnvironment
		bDynamic=FALSE
		bCastShadows=FALSE
		AmbientGlow=(R=0.3,G=0.3,B=0.3,A=1.0)
		TickGroup=TG_DuringAsyncWork
	End Object
	MyLightEnvironment=DroppedPickupLightEnvironment
	Components.Add(DroppedPickupLightEnvironment)


	Begin Object NAME=CollisionCylinder
		CollisionRadius=+00050.000000
		CollisionHeight=+00020.000000
		CollideActors=true
	End Object

	PickupAction={(
			 ActionName=DroppedPickupIcon,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=360,V=0,UL=58,VL=55))),
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=156,V=233,UL=34,VL=56)))	),
			)}

	NoSymbolIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=411,V=342,UL=49,VL=49)

	bCollisionSoundsEnabled=TRUE
}
