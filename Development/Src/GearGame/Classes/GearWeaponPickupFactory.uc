/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeaponPickupFactory extends GearPickupFactory
	native;


var() RepNotify class<GearWeapon>	WeaponPickupClass;
var				class<GearWeapon>	OriginalPickupClass;
/** Keep track of which mesh was set, to not create it twice. */
var				class<Actor>		CachedMeshActorClass;

/** When using a non-gearweapon (shield) this is a reference to that class */
var class<Inventory> PlaceholderClass;

var const		ActionInfo			CantPickupAction;

var() const array< class<Inventory> > MPWeaponPickupList;

/** Whether to exclude this spawner for Annex gameplay */
var() bool bMP_ExcludeAnnex;
/** Whether to exclude this spawner for KOTH gameplay */
var() bool bMP_ExcludeKOTH;
/** Whether to exclude this spawner for Meatflag gameplay */
var() bool bMP_ExcludeMeatflag;

var() int ExtraAmmoCount;

/** extra translation to apply to the visual mesh, for when the pickup is on uneven terrain and such
 * since moving the nav point itself could break the AI
 */
var() vector MeshTranslation;

cpptext
{
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

replication
{
	if (ROLE==ROLE_Authority)
		WeaponPickupClass, PlaceholderClass;
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'WeaponPickupClass' )
	{
		PickupClassChanged();
		if (WeaponPickupClass != None && IsInState('Disabled'))
		{
			GotoState('Auto');
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** Internal. */
protected function TriggerPickupGUDS(GearPawn GP, GearWeapon NewWeap)
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

simulated function PreBeginPlay()
{
	// Store original pickup classes for gametypes that depend on it (ie Annex)
	OriginalPickupClass = WeaponPickupClass;
	PickupClassChanged();

	Super.PreBeginPlay();
}

simulated function PickupClassChanged()
{
	InventoryType = WeaponPickupClass;
	SetPickupMesh();
}

/** Overridden to use DroppedPickupMesh instead of PickupFactoryMesh */
simulated function SetPickupMesh()
{
	local PrimitiveComponent	InvPickupMesh;
	local SkeletalMeshComponent	SkelMeshComp;
	local class<Actor>			MeshActorClass;

	if( InventoryType == None )
	{
		if( PickupMesh != None || CachedMeshActorClass != None )
		{
			// clear pickup mesh.
			DetachComponent(PickupMesh);
			PickupMesh = None;
			CachedMeshActorClass = None;
		}
	}
	else
	{
		if( WeaponPickupClass != None )
		{
			InvPickupMesh = WeaponPickupClass.static.GetPickupMesh(self);
			MeshActorClass = WeaponPickupClass;
		}

		if( InvPickupMesh == None )
		{
			InvPickupMesh = InventoryType.default.DroppedPickupMesh;
			MeshActorClass = InventoryType;
		}

		// If mesh needs to be updated, do it!
		if( InvPickupMesh != None && (PickupMesh == None || MeshActorClass != CachedMeshActorClass) )
		{
			if( PickupMesh != None )
			{
				DetachComponent(PickupMesh);
				PickupMesh = None;
			}

			PickupMesh = new(self) InvPickupMesh.Class(InvPickupMesh);
			PickupMesh.SetLightEnvironment( MyLightEnvironment );
			PickupMesh.SetHidden(bPickupHidden);
			PickupMesh.SetTranslation(InvPickupMesh.Translation + MeshTranslation);
			AttachComponent(PickupMesh);
			CachedMeshActorClass = MeshActorClass;

			SkelMeshComp = SkeletalMeshComponent(PickupMesh);
			if( SkelMeshComp != None )
			{
				// Disable animations on pickups, they don't need those.
				SkelMeshComp.SetAnimTreeTemplate(None);
				SkelMeshComp.bForceRefpose = 1;
				SkelMeshComp.bUpdateSkelWhenNotRendered = FALSE;
			}

			// this component is destroyed when picked up so we don't need to worry about changing
			// the emissive color back to the default
			CreateAndSetMICWeaponSkin(MeshComponent(PickupMesh));
			// all weapons should have the EmissiveCOG as their colors
		}
	}
}

auto state Pickup
{
	/**
	* Validate touch (if valid return true to let other pick me up and trigger event).
	* Overridden to support GearPawn.bCanPickupFactoryWeapons.
	*/
	simulated function bool ValidTouch( Pawn Other )
	{
		local GearHUD_Base WHUD;
		if ( (GearPawn(Other) != None) && !GearPawn(Other).bCanPickupFactoryWeapons )
		{
			if ( PlayerController(Other.Controller) != None )
			{
				WHUD = GearHUD_Base(PlayerController(Other.Controller).MyHUD);
				if ( WHUD != None )
					WHUD.WeaponFactoryPickupFailed();
			}
			return false;
		}

		return Super.ValidTouch(Other);
	}

	event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
	{
		local GearAI_TDM Bot;
		local GearPawn P;

		Super.Touch(Other, OtherComp, HitLocation, HitNormal);

		// if a bot touched me while doing something else, see if it wants to pick me up anyway
		if (InventoryType != None)
		{
			P = GearPawn(Other);
			if (P != None && P.CanDoSpecialMove(SM_WeaponPickup))
			{
				Bot = GearAI_TDM(P.Controller);
				if (Bot != None && Bot.MoveTarget != self && Bot.AllowDetourToPickUp(InventoryType))
				{
					class'AICmd_PickupWeapon'.static.PickupWeaponFromFactory(Bot, self);
				}
			}
		}
	}
}

function bool CheckForErrors()
{
	if( super.CheckForErrors() )
	{
		return TRUE;
	}

	if( WeaponPickupClass == None )
	{
		`log(Self @ "no weapon pickup class");
		return TRUE;
	}

	return FALSE;
}

event Reset()
{
	Super.Reset();
	bPickupHidden = FALSE;
	SetPickupMesh();
}

State Disabled
{
	/** Overridden to support checkpoint loading */
	function Reset()
	{
		SetPickupVisible();
		SetCollision(TRUE,FALSE);
		Global.Reset();
	}
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

	if ( bHidden || (PickupMesh == None) || PickupMesh.HiddenGame )
	{
		return '';
	}

	if ( (GearPawn(P) != None) && !GearPawn(P).bCanPickupFactoryWeapons )
	{
		return 'PickupLocked';
	}

	// Don't allow any pickup when carrying crate
	if(GearPawn(P) != None && GearPawn(P).CarriedCrate != None)
	{
		return 'None';
	}

	InvManager = GearInventoryManager(P.InvManager);

	// if we're already carrying that same weapon
	if (!ClassIsChildOf(WeaponPickupClass, class'GearWeap_HeavyBase') && InvManager.FindInventoryType(WeaponPickupClass, TRUE) != None)
	{
		// check if we can pick up ammo from it
		if (CanPickUpAmmoType(P, WeaponPickupClass.default.AmmoTypeClass, bExcludeChecks))
		{
			return 'TakeAmmo';
		}
	}
	else
	{
		// See if we can find an available slot for this weapon
		DesiredSlot = InvManager.FindFreeSlotForInventoryClass(WeaponPickupClass);

		if( DesiredSlot != EASlot_None )
		{
			//`log( "0" );
			// we found a free slot, we can pickup weapon!
			return 'PickUp';
		}

		// if shoulder weapon then do kick up
		if( InvManager.CanSwapWeaponClassWithCurrent(WeaponPickupClass) && P.CanThrowWeapon() )
		{
			//`log( "2" );
			return 'SwapWithCurrent';
		}
		else if ( bExcludeChecks && WeaponPickupClass.default.WeaponType == WT_Normal && GearWeapon(InvManager.Instigator.Weapon).WeaponType != WT_Normal )
		{
			return 'SwapWithCurrent';
		}

		// If we want to pick up a pistol or a grenade, we can just grab and exchange it
		if( WeaponPickupClass.default.WeaponType == WT_Holster ||
			WeaponPickupClass.default.WeaponType == WT_Item )
		{
			//`log( "3" );
			return 'SwapWithAttachment';
		}

		// for heavy weapons, we pick up and hold
		if (WeaponPickupClass.default.WeaponType == WT_Heavy)
		{
			if ( (GearWeapon(P.Weapon) != None) && (GearWeapon(P.Weapon).WeaponType == WT_Heavy) )
			{
				return 'SwapCarried';
			}
			else
			{
				return 'PickUpAndCarry';
			}
		}
	}

	return '';
}

/** this function has been made obsolete by GiveTo(). */
function SpawnCopyFor( Pawn Recipient );

function PickedUpBy(Pawn P)
{
	local GearAI_TDM Bot;
	local AICmd_MoveToGoal MoveCmd;

	Super.PickedUpBy(P);

	// interrupt any bots that were also going for this pickup as it is no longer available
	foreach WorldInfo.AllControllers(class'GearAI_TDM', Bot)
	{
		if (Bot.Pawn != None && Bot.Pawn != P && Bot.RoamGoal == self && Bot.MoveGoal == self)
		{
			MoveCmd = Bot.FindCommandOfClass(class'AICmd_MoveToGoal');
			if (MoveCmd != None)
			{
				`AILog_Ext("Aborting move to" @ self @ "because taken by another player", 'None', Bot);
				Bot.AbortCommand(MoveCmd);
			}
		}
	}
}

/**
 * Give pickup to player
 */
function GiveTo( Pawn P )
{
	local GearInventoryManager	InvManager;
	local GearWeapon				NewWeap, OtherWeapon;
	local SoundCue AmmoPickupSound;
	local GearPawn GP;
	local name Interaction;

	GP = GearPawn(P);

	InvManager	= GearInventoryManager(P.InvManager);
	Interaction = FindInteractionWith(P, false);

	switch( Interaction )
	{
		/** just take ammo from weapon */
		case 'TakeAmmo' :
			// Fill up ammo if we can. Otherwise leave weapon.
			if( PickUpAmmoFromWeaponClass(P, WeaponPickupClass) )
			{
				AmmoPickupSound = WeaponPickupClass.default.AmmoTypeClass.default.AmmoPickupSound;
				//DupWeap.AnnouncePickup(P);
				//P.PlaySound( AmmoPickupSound );
				if (GP != None)
				{
					GP.AmmoPickupSound = AmmoPickupSound;
				}
				PickedUpBy(P);
			}
			break;

		/** throw current weapon, and pick up new one */
		case 'SwapWithCurrent' :
			//@note: intentionally not using ThrowActiveWeapon() as that would cause a SwitchToBestWeapon() we don't want
			P.TossInventory(P.Weapon);

			NewWeap = GearWeapon(InvManager.CreateInventory(WeaponPickupClass));

			// Switch to new weapon
			InvManager.SetCurrentWeapon(NewWeap);

			//NewWeap.AnnouncePickup(P);
			TriggerPickupGUDS(GP, NewWeap);
			P.PlaySound( NewWeap.PickupSound );
			PickedUpBy(P);
			NewWeap.AddAmmoMessage( NewWeap.GetPickupAmmoAmount() + ExtraAmmoCount );

			if (ExtraAmmoCount > 0)
			{
				NewWeap.AddAmmo(ExtraAmmoCount);
			}

			// check to see if we are an MP game
			if( GearGRI(WorldInfo.GRI).IsMultiPlayerGame() == TRUE )
			{
				GearGame(WorldInfo.Game).GearBroadcastWeaponTakenMessage( P.Controller, NewWeap.default.DamageTypeClassForUI );
			}

			break;

		/** Just pick up new weapon */
		case 'PickUp' :
			NewWeap = GearWeapon(InvManager.CreateInventory(WeaponPickupClass));

			TriggerPickupGUDS(GP, NewWeap);
			P.PlaySound( NewWeap.PickupSound );
			PickedUpBy(P);
			NewWeap.AddAmmoMessage( NewWeap.GetPickupAmmoAmount() + ExtraAmmoCount );

			if (ExtraAmmoCount > 0)
			{
				NewWeap.AddAmmo(ExtraAmmoCount);
			}

			// check to see if we are an MP game
			if( GearGRI(WorldInfo.GRI).IsMultiPlayerGame() == TRUE )
			{
				GearGame(WorldInfo.Game).GearBroadcastWeaponTakenMessage( P.Controller, NewWeap.default.DamageTypeClassForUI );
			}

			// Switch to that new weapon only if it's not a grenade.
			if( NewWeap != None && !NewWeap.IsA('GearWeap_GrenadeBase') )
			{
				InvManager.SetCurrentWeapon(NewWeap);
			}

			break;

		// Take weapon and Swap it with an attachment (ie not the currently held one)
		case 'SwapWithAttachment' :

			// Toss carried weapon
			OtherWeapon = InvManager.FindWeaponOfType(WeaponPickupClass.default.WeaponType);
			//`log("OtherWeapon:" @ OtherWeapon);
			P.TossInventory(OtherWeapon);

			// Grab new one
			NewWeap = GearWeapon(InvManager.CreateInventory(WeaponPickupClass));

			//NewWeap.AnnouncePickup(P);
			TriggerPickupGUDS(GP, NewWeap);
			P.PlaySound( NewWeap.PickupSound );
			PickedUpBy(P);
			NewWeap.AddAmmoMessage( NewWeap.GetPickupAmmoAmount() + ExtraAmmoCount );

			if (ExtraAmmoCount > 0)
			{
				NewWeap.AddAmmo(ExtraAmmoCount);
			}

			// check to see if we are an MP game
			if( GearGRI(WorldInfo.GRI).IsMultiPlayerGame() == TRUE )
			{
				GearGame(WorldInfo.Game).GearBroadcastWeaponTakenMessage( P.Controller, NewWeap.default.DamageTypeClassForUI );
			}

			// Switch to that new weapon only if it's not a grenade.
			if( NewWeap != None && !NewWeap.IsA('GearWeap_GrenadeBase') )
			{
				InvManager.SetCurrentWeapon(NewWeap);
			}

			break;


		case 'SwapCarried':
			P.ThrowActiveWeapon();
			// intentional fall-through

		/** Pick up and hold in hands. */
		case 'PickUpAndCarry':

			NewWeap = GearWeapon(InvManager.CreateInventory(WeaponPickupClass));

			P.PlaySound( NewWeap.PickupSound );
			PickedUpBy(P);

			if (ExtraAmmoCount > 0)
			{
				NewWeap.AddAmmo(ExtraAmmoCount);
			}

			// check to see if we are an MP game
			if( GearGRI(WorldInfo.GRI).IsMultiPlayerGame() == TRUE )
			{
				GearGame(WorldInfo.Game).GearBroadcastWeaponTakenMessage( P.Controller, NewWeap.default.DamageTypeClassForUI );
			}

			InvManager.SetCurrentWeapon(NewWeap);

			if (GP != None)
			{
				GP.SoundGroup.PlayEffort(GP, GearEffort_LiftHeavyWeaponEffort);
			}
			break;

		case 'PickupLocked':
			break;

		default:
			`log( "GearWeaponPickupFactory.GiveTo() has failed with: " $ FindInteractionWith(P,false) );
			ScriptTrace();
			break;
	}
}


simulated function bool GetPickupAction(out ActionInfo out_PickupAction, Pawn P)
{
	local Name Interaction;

	if ( bHidden || (PickupMesh == None) || PickupMesh.HiddenGame )
	{
		return FALSE;
	}

	Interaction = FindInteractionWith(P,true);
	if ( Interaction == 'PickupLocked' )
	{
		out_PickupAction = CantPickupAction;
		out_PickupAction.ActionIconDatas[2].ActionIcons[0] = WeaponPickupClass.default.WeaponIcon;
		return TRUE;
	}
	else if( Super.GetPickupAction(out_PickupAction,P) )
	{
		// figure out what icons need to swap
		out_PickupAction.ActionName = Name;

		if ( Interaction != '' )
		{
			if ( Interaction != 'TakeAmmo' )
			{
				// replace the ammo icon with weapon icon
				out_PickupAction.ActionIconDatas[2].ActionIcons[0] = WeaponPickupClass.default.WeaponIcon;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Overridden to check for unique weapon classes, preventing respawn if the weapon exists somewhere else in the level.
 */
function bool DelayRespawn()
{
	local bool bUnique;
	local GearDroppedPickup DroppedPickup;
	local GearPawn Pawn;
	local GearWeapon Weapon;
	local Actor A;
	local int NumFactories;
	local GearWeaponPickupFactory Factory;
	// if this weapon pickup is unique don't respawn if one already exists
	bUnique = WeaponPickupClass.default.bUniqueSpawn;
	if (bUnique)
	{
		foreach WorldInfo.AllNavigationPoints(class'GearWeaponPickupFactory', Factory)
		{
			if (Factory.WeaponPickupClass == WeaponPickupClass && !Factory.IsInState('Pickup'))
			{
				NumFactories++;
			}
		}
		foreach DynamicActors(class'Actor', A)
		{
			DroppedPickup = GearDroppedPickup(A);
			if (DroppedPickup != None &&
				!DroppedPickup.bHidden &&
				DroppedPickup.InventoryClass == WeaponPickupClass)
			{
				NumFactories--;
			}
			else
			{
				Pawn = GearPawn(A);
				if (Pawn != None &&
					Pawn.InvManager != None)
				{
					Weapon = GearWeapon(Pawn.InvManager.FindInventoryType(WeaponPickupClass,FALSE));
					if (Weapon != None && Weapon.HasAnyAmmo())
					{
						NumFactories--;
					}
				}
			}
		}
		return (NumFactories == 0);
	}
	return Super.DelayRespawn();
}

simulated event string GetDebugAbbrev()
{
	return "WPF";
}


simulated protected function bool CanPickUpWhileHoldingHeavyWeapon(name Interaction)
{
	 if (Interaction == 'PickUp' && ClassIsChildOf(WeaponPickupClass, class'GearWeap_GrenadeBase'))
	 {
		 return TRUE;
	 }

	 // ok to swap from heavy to heavy
	 if (Interaction == 'SwapWithCurrent' && WeaponPickupClass.default.WeaponType == WT_Heavy)
	 {
		return TRUE;
	 }

	 return super.CanPickUpWhileHoldingHeavyWeapon(Interaction);
}


/** This is a function to allow subclasses to return an emissive LinearColor. (e.g. some weapons need to be colored and others don't) **/
simulated function LinearColor GetEmissiveColorParam()
{
	local LinearColor LC;

	// so for all weapons EXCEPT grenades we want to not have any coloring for weapon pickups
	if( ( WeaponPickupClass != none )
		&&( ( WeaponPickupClass.default.WeaponID == WC_FragGrenade )
		     || ( WeaponPickupClass.default.WeaponID == WC_GasGrenade )
			 || ( WeaponPickupClass.default.WeaponID == WC_InkGrenade )
			 || ( WeaponPickupClass.default.WeaponID == WC_SmokeGrenade )
			 )
		)
	{
		LC = WeaponPickupClass.static.GetWeaponEmisColor_COG( WorldInfo, WeaponPickupClass.default.WeaponID );
		//`log( "using global EmissiveCOG " @ WeaponPickupClass.default.WeaponID @  LC.A @ LC.R @ LC.G @ LC.B );
	}

	// else we want to return a "blank' LC

	return LC;
}



defaultproperties
{
	//bStatic=FALSE
	bNoDelete=TRUE

	PickupAction={(
			 ActionName=WeaponPickupIcon,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=360,V=0,UL=58,VL=55))),
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=156,V=233,UL=34,VL=56)))	),
			)}

	CantPickupAction={(
			 ActionName=WeaponNoPickupIcon,
			 ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=411,V=342,UL=49,VL=49))), // No entry
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=360,V=0,UL=58,VL=55))),
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=156,V=233,UL=34,VL=56)))	),
			)}
}
