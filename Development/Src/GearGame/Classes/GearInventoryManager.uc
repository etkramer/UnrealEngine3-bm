/**
 * GearInventoryManager
 * Gear inventory definition
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearInventoryManager extends InventoryManager
	config(Weapon)
	DependsOn(GearTypes,GearWeapon);

/**
 * Player Accuracy modifiers.
 */
// Base Accuracy values
/** Accuracy: Standing */
var()	config float		Base_Acc_Standing;
/** Accuracy: Blind firing */
var()	config float		Base_Acc_BlindFire;
/** Accuracy modifier: targeting mode */
var()	config float		Base_Acc_Target;

// Accuracy modifiers
/** Accuracy modifier: moving */
var()	config float		mod_Acc_Move;
/** Accuracy modifier: evading (added on top of moving) */
var()	config float		mod_Acc_Evade;
/** Accuracy modifier: view turning */
var()	config float		mod_Acc_ViewTurn;


// Base Interp value
/** Interp speed: default */
var()	config float		Base_Interp_Speed;

// Interp modifiers
/** Interp modifier: blind fire */
var()	config float		mod_Interp_BlindFire;
/** Interp modifier: movement */
var()	config float		mod_Interp_Move;
/** Interp modifier: evading (added on top of movement) */
var()	config float		mod_Interp_Evade;
/** Interp scaler: view turning */
var()	config float		mul_Interp_ViewTurn;

// Boundaries
/** Interp speed boundaries */
var()	config vector2d		InterpRange;


/** Player Accuracy. Do not read/write, use accessor function GetPlayerNormalizedAccuracy() */
var				float			Accuracy;
/** last player view rotation, to affect accuracy */
var	transient	rotator			AccuracyLastViewRot;


/** true if player is in weapon selection interface */
var				bool	bShowWeaponSlots;
/** time out after which the weapon selection screen closes */
var				float	WeapSelHUDTimeOut;
/** counter for the above */
var				float	WeapSelHUDTimeCount;
/** Speed multiplier of how fast the weapon indicators will fade out */
var				float	WeapSelHUDTimeCountSpeedMult;

/** which weapon slot is currently highlighted */
var				GearPawn.EAttachSlot HighlightedWeaponSlot;

/** Reference to the HUD */
var				HUD		TheHUD;

/** The linear order of the inventory slots for weapon switching */
var array<GearPawn.EAttachSlot> LinearSlotOrder;

/** TRUE to allow >4 weapons to be carried at once.  For debug only. */
var				bool	bDebugIgnoreWeaponCountLimit;

/** Weapon we had equipped before the current one. */
var transient GearWeapon PreviousEquippedWeapon;

/** Reference to any shield we're carrying. */
var transient GearShield Shield;

/** At what absolute pitch to toss dropped inventory items. */
var() const int InventoryTossPitch;
/** At what speed to toss dropped inventory items. */
var() const int InventoryTossSpeed;


/**
 * We override this so we do not auto switch weapons when we drop a weapon.
 *
 * NOTE:  IF we want this type of functionality we need to pass in a contextObject
 *        with something like:  rangeOfTarget, targetResistences etc.  So we can
 *        decide wether to use a sniper rifle or a shotgun.  A single "goodness" rating
 *        is not enough.
 **/
simulated function SwitchToBestWeapon( optional bool bForceADifferentWeapon );

simulated protected function Weapon DecideWhichWeaponToAutoSwitchTo()
{
	local GearWeapon WeapRight;
	local GearWeapon WeapLeft;
	local GearWeapon WeapBelt;
	local GearPawn GP;

	GP = GearPawn(Owner);

	if (GP.IsCarryingShield())
	{
		// favor one-handed (e.g. pistol)
		WeapBelt = GearWeapon(GetInventoryInSlot( EASlot_Holster ));
		if( (WeapBelt != None) && WeapBelt.HasAnyAmmo() == TRUE )
		{
			return WeapBelt;
		}
	}

	// if currently wielding a heavy or grenade, return to previous weapon
	if( PreviousEquippedWeapon != None &&
		(GP.MyGearWeapon.WeaponType == WT_Heavy || GP.MyGearWeapon.WeaponType == WT_Item) &&
		PreviousEquippedWeapon.CharacterSlot != EASlot_None &&
		PreviousEquippedWeapon.HasAnyAmmo() )
	{
		return PreviousEquippedWeapon;
	}

	// else check the slots in priority order
	WeapRight = GearWeapon(GetInventoryInSlot( EASlot_RightShoulder ));
	if( (WeapRight != None) && WeapRight.HasAnyAmmo() == TRUE )
	{
		return WeapRight;
	}

	WeapLeft = GearWeapon(GetInventoryInSlot( EASlot_LeftShoulder ));
	if( (WeapLeft != None) && WeapLeft.HasAnyAmmo() == TRUE )
	{
		return WeapLeft;
	}

	WeapBelt = GearWeapon(GetInventoryInSlot( EASlot_Holster ));
	if( (WeapBelt != None) && WeapBelt.HasAnyAmmo() == TRUE )
	{
		return WeapBelt;
	}

	if( (WeapRight != None) && WeapRight.HasSpareAmmo() == TRUE )
	{
		return WeapRight;
	}

	if( (WeapLeft != None) && WeapLeft.HasSpareAmmo() == TRUE )
	{
		return WeapLeft;
	}

	if( (WeapBelt != None) && WeapBelt.HasSpareAmmo() == TRUE )
	{
		return WeapBelt;
	}

	// we have failed to find any weapons with ammo or spare ammo so switch to the assault rifle
	return WeapRight;
}

simulated function AutoSwitchWeapon()
{
	local Weapon W;

	if (WorldInfo.NetMode != NM_Client || (Instigator != None && Instigator.IsLocallyControlled()))
	{
		W = DecideWhichWeaponToAutoSwitchTo();
		`LogInv("Found Weapon:" @ W @ "CurrentWeapon:" @ Instigator.Weapon );
		if (W != None)
		{
			SetCurrentWeapon(W);
		}
		else
		{
			NextWeapon();
		}
	}
}

/** @see InventoryManager::StartFire */
simulated function StartFire(byte FireModeNum)
{
	local GearPawn GP;
	GP = GearPawn(Owner);
	if( GP != None )
	{
		// mark the last time the weapon was fired for AI use
		GP.LastWeaponStartFireTime = WorldInfo.TimeSeconds;

		if( GearPC(GP.Controller) != None )
		{
			GearPC(GP.Controller).SetLastWeaponInfoTime(WorldInfo.TimeSeconds);
		}
	}

	super.StartFire(FireModeNum);
}


/**
 * Switches to Previous weapon
 * Network: Client
 */
simulated function PrevWeapon()
{
	if( GearWeapon(Pawn(Owner).Weapon) != None && GearWeapon(Pawn(Owner).Weapon).DoOverridePrevWeapon() )
	{
		return;
	}

	super.PrevWeapon();
}

/**
 *	Switches to Next weapon
 *	Network: Client
 */
simulated function NextWeapon()
{
	if( GearWeapon(Pawn(Owner).Weapon) != None && GearWeapon(Pawn(Owner).Weapon).DoOverrideNextWeapon() )
	{
		return;
	}

	super.NextWeapon();
}

/** Get the next slot for weapon switching */
simulated function GearPawn.EAttachSlot GetNextSlot( GearPawn.EAttachSlot CurSlot )
{
	return GetAdjacentSlot( CurSlot, 1 );
}

/** Get the previous slot for weapon switching */
simulated function GearPawn.EAttachSlot GetPrevSlot( GearPawn.EAttachSlot CurSlot )
{
	return GetAdjacentSlot( CurSlot, -1 );
}

/** Finds the next or previous slot for weapon switching without wrapping */
simulated function GearPawn.EAttachSlot GetAdjacentSlot( GearPawn.EAttachSlot CurSlot, int Dir )
{
	local int CurIdx, Idx;

	CurIdx = Max( LinearSlotOrder.Find( CurSlot ), 0);
	Idx = CurIdx;

	do
	{
		// Don't wrap to other side of the menu
		Idx += Dir;
		if( (Idx < 0) || (Idx >= LinearSlotOrder.Length) )
		{
			return LinearSlotOrder[CurIdx];
		}

		if( GetInventoryInSlot(LinearSlotOrder[Idx]) != None )
		{
			return LinearSlotOrder[Idx];
		}
	}
	until ( Idx == CurIdx );


	return LinearSlotOrder[CurIdx];
}

/**
 * Forces to reload current weapon
 * i.e change current magazine with a new one
 */
exec simulated function ForceReloadWeapon()
{
	if( GearWeapon(Pawn(Owner).Weapon) != None )
	{
		GearWeapon(Pawn(Owner).Weapon).ForceReload();
	}
}

/**
 * Called from PlayerController::UpdateRotation() -> PlayerController::ProcessViewRotation() -> Pawn::ProcessViewRotation()
 * to (pre)process player ViewRotation.
 * adds delta rot (player input), applies any limits and post-processing
 * returns the final ViewRotation set on PlayerController
 *
 * @param	DeltaTime, time since last frame
 * @param	ViewRotation, actual PlayerController view rotation
 * @input	out_DeltaRot, delta rotation to be applied on ViewRotation. Represents player's input.
 * @return	processed ViewRotation to be set on PlayerController.
 */

simulated function ProcessViewRotation( float DeltaTime, out rotator out_ViewRotation, out Rotator out_DeltaRot )
{
	local Weapon	ActiveWeapon;

	if (Pawn(Owner) != None)
	{
		// Give weapon a chance to modify playercontroller's viewrotation
		ActiveWeapon = Pawn(Owner).Weapon;
		if( GearWeapon(ActiveWeapon) != None )
		{
			GearWeapon(ActiveWeapon).ProcessViewRotation( DeltaTime, out_ViewRotation, out_DeltaRot );
		}
	}
}


simulated function Tick( float DeltaTime )
{
	`if(`notdefined(FINAL_RELEASE))
	local GearPawn	P;
	`endif

	super.Tick( DeltaTime );

	// disable accuracy for AI
	if( Instigator != None &&
		PlayerController(Instigator.Controller) != None )
	{
		// force accuracy update every tick.
		UpdatePlayerAccuracy( DeltaTime );

		`if(`notdefined(FINAL_RELEASE))
		P = GearPawn(Instigator);
		if( P != None && P.bWeaponDebug_Accuracy )
		{
			DebugDrawWeaponAccuracy();
		}
		`endif
	}
}

simulated function DebugDrawWeaponAccuracy()
{
	local float			AngleRad;
	local GearWeapon		W;
	local vector		StartTrace, EndTrace, Dir;

	W = GearWeapon(Instigator.Weapon);
	if( W != None )
	{
		AngleRad	= Sin(W.GetPlayerAimError() * Pi/180.f);

		StartTrace	= Instigator.GetWeaponStartTraceLocation();
		EndTrace	= StartTrace + vector(Instigator.GetAdjustedAimFor(W, StartTrace)) * W.GetTraceRange();
		Dir = Normal(EndTrace - StartTrace);

		// slight nudge, in case starttrace is at the camera, makes things look more... sane.
		StartTrace += Dir * 20;

		DrawDebugCone(StartTrace, Dir, W.GetTraceRange(), AngleRad, AngleRad, 16, MakeColor(255,128,64,255), FALSE);
	}
}


simulated function UpdatePlayerAccuracy( float DeltaTime )
{
	local float		InaccuracyPct, interpSpeed, RotRateRatio, SpeedRatio;
	local GearPawn	P;
	local Rotator	DeltaRot, ViewRot;
	local Vector	ViewLoc;

	if( DeltaTime == 0.f )
	{
		return;
	}

	interpSpeed	= Base_Interp_Speed;

	P = GearPawn(Instigator);

	if( P == None || P.Weapon == None )
	{
		return;
	}

	// Targeting Mode
	if( P.bIsTargeting )
	{
		InaccuracyPct = Base_Acc_Target;
	}
	else if( P != None && P.CoverType != CT_None )
	{
		if( P.CoverAction == CA_LeanLeft ||
			P.CoverAction == CA_LeanRight ||
			P.CoverAction == CA_PopUp )
		{
			// standing cover
			InaccuracyPct = Base_Acc_Standing;
		}
		else
		{
			// blind fire
			InaccuracyPct	= Base_Acc_BlindFire * GearWeapon(P.Weapon).Base_AccScale_BlindFire;
			interpSpeed += mod_Interp_BlindFire;
		}
	}
	else
	{
		// standing
		InaccuracyPct = Base_Acc_Standing;
	}

	// Movement
	SpeedRatio = VSize(Instigator.Velocity) / Instigator.GroundSpeed;
	if( SpeedRatio > 0 )
	{
		InaccuracyPct	+= SpeedRatio * mod_Acc_Move;
		interpSpeed		+= SpeedRatio * mod_Interp_Move;
	}

	// Evade
	if( P != None && P.IsEvading() )
	{
		InaccuracyPct	+= mod_Acc_Evade;
		interpSpeed		+= mod_Interp_Evade;
	}

	Instigator.GetActorEyesViewPoint( ViewLoc, ViewRot );
	DeltaRot = ViewRot - AccuracyLastViewRot;
	AccuracyLastViewRot = ViewRot;

	// ViewRotation modifier
	if( AccuracyLastViewRot == Rot(0,0,0) )
	{
		AccuracyLastViewRot = ViewRot;
	}

	if( DeltaRot != rot(0,0,0) )
	{
		RotRateRatio	= RSize( DeltaRot ) / 1024;
		InaccuracyPct	+= RotRateRatio * mod_Acc_ViewTurn;
		interpSpeed		+= RotRateRatio * RotRateRatio * mul_Interp_ViewTurn;
		//`log("RotRateRatio:" @ RotRateRatio);
	}

	// Clamp interp speed
	interpSpeed	= fClamp(interpSpeed, InterpRange.X, InterpRange.Y);
	// interpolated accuracy, and make sure it is withing boundaries (0=best, 1=worst)
	Accuracy	= FClamp(FInterpTo( Accuracy, InaccuracyPct, DeltaTime, interpSpeed ), 0.f, 1.f );

	//`log( "Accuracy:" $ Accuracy @ "InaccuracyPct:" $ InaccuracyPct @ "interpSpeed:" $ interpSpeed );
}


/**
 * Returns a modifier (scalar) that defines player accuracy.  Note that this is a value
 * in the range [0..1] denoting "most accurate" to "least accurate" (hence normalized accuracy).
 * Actual accuracy is computed using this in subsequent code.
 * Modifier is defined by several parameters such as player speed, posture, targetting mode etc..
 * Accuracy is then used for weapon firing and other things like affecting recoil.
 *
 * @return	Accuracy, float value. 0 = perfect aim, 1.f worst aim.
 */
simulated function float GetPlayerNormalizedAccuracy()
{
	// disable accuracy for AI.
	if( PlayerController(Instigator.Controller) == None )
	{
		return 0.f;
	}

	return Accuracy;
}


/** Hook called from HUD actor. Gives access to HUD and Canvas */
simulated function DrawHUD( HUD H )
{
	// do not draw inventory HUD if we're not viewing our character.
	if ( (Owner == None) ||
		(PlayerController(Pawn(Owner).Controller) == None) ||
		(Owner != PlayerController(Pawn(Owner).Controller).ViewTarget) )
	{
		return;
	}

	//H.Canvas.SetPos( 200, 200 );
	//H.Canvas.DrawText( "Recoil"@WeaponRecoilOffset@"AutoCorrect"@AutoCorrectWeaponRecoilOffset );

	super.DrawHUD( H );
	DrawWeaponSlots( H );
	TheHUD = H;
}


/** Draw Weapon Slots during selection */
simulated function DrawWeaponSlots( HUD H )
{
	local GearHUD_Base	wfHUD;
	local float		AlphaPct;

	// update fade out timer
	if (!bShowWeaponSlots)
	{
		// already faded out
		if (WeapSelHUDTimeCount == -1)
		{
			// don't bother rendering
			return;
		}
		// otherwise increment the counter
		WeapSelHUDTimeCount += H.RenderDelta * WeapSelHUDTimeCountSpeedMult;
		if( WeapSelHUDTimeCount >= WeapSelHUDTimeOut )
		{
			WeapSelHUDTimeCount	= -1;
		}
	}
	AlphaPct = (WeapSelHUDTimeCount / WeapSelHUDTimeOut);
	AlphaPct = 1.f - AlphaPct * AlphaPct;
	wfHUD = GearHUD_Base(H);

	DrawCircularWeaponSlots( wfHUD, AlphaPct );
}

/** Draw the keyboard/mouse straight vertical line selection UI */
simulated function DrawLinearWeaponSlots( GearHUD_Base wfHUD, float AlphaPct )
{
	local GearPC	PC;
	local GearPawn	WP;
	local GearWeapon Weap;
	local byte 		Alpha;
	local bool		bHighlight;
	local float		DrawX, DrawY, FrameSizeY;
	local float		WeaponCenterX, WeaponCenterY, WeaponScale;
	local int		Idx;
	local float		ResScaleSmall;

	if( wfHUD == None )
		return;

	ResScaleSmall = 1.0f;
	WeaponScale = 0.85f;
	PC = GearPC(Instigator.Controller);
	WP = GearPawn(Instigator);

	CalculateWeaponSlotCenter( WeaponCenterX, WeaponCenterY, wfHUD, WeaponScale*ResScaleSmall );

	FrameSizeY = (wfHUD.WeaponInfoFrame.VL/2)*WeaponScale*ResScaleSmall;
	DrawY = WeaponCenterY - FrameSizeY ;
	DrawX = 16.f*ResScaleSmall;

	for( Idx = 0; Idx < LinearSlotOrder.length; Idx++ )
	{
		Weap = GearWeapon(GetInventoryInSlot(LinearSlotOrder[Idx]));
		if (Weap != None)
		{
			bHighlight = (HighlightedWeaponSlot == LinearSlotOrder[Idx]) ? TRUE : FALSE;
			Alpha = 255 * AlphaPct * (Weap.HasAnyAmmo() ? 1.f : 0.5f);
			wfHUD.DrawWeaponInfo(PC, WP, Weap, DrawX, DrawY, Alpha, bHighlight, WeaponScale);
		}
		else
		{
			Alpha = 255 * AlphaPct * 0.3f;
			wfHUD.DrawWeaponInfo(PC, WP, None, DrawX, DrawY, Alpha, false, WeaponScale, false, true);
		}

		DrawY += (FrameSizeY * 2.f) + 16.f*ResScaleSmall;
	}

}

/** Draw the console circular weapon selection UI */
simulated function DrawCircularWeaponSlots( GearHUD_Base wfHUD, float AlphaPct )
{
	local GearPC		PC;
	local GearPawn	WP;
	local GearWeapon Weap;
	local byte 		Alpha;
	local bool		bHighlight;
	local float		WeaponCenterX, WeaponCenterY, WeaponScale;
	local float		ResScaleSmall;

	if( wfHUD == None )
		return;

	ResScaleSmall = 1.0f;
	WeaponScale = 0.85f;
	PC = GearPC(Instigator.Controller);
	WP = GearPawn(Instigator);

	CalculateWeaponSlotCenter( WeaponCenterX, WeaponCenterY, wfHUD, WeaponScale*ResScaleSmall );

	Weap = GearWeapon(GetInventoryInSlot(EASlot_Belt));
	if (Weap != None)
	{
		bHighlight = (HighlightedWeaponSlot == EASlot_Belt) ? TRUE : FALSE;
		Alpha = 255 * AlphaPct * (Weap.HasAnyAmmo() ? 1.f : 0.5f);
		wfHUD.DrawWeaponInfo(PC,WP,Weap,WeaponCenterX - (wfHUD.WeaponInfoFrame.UL*ResScaleSmall/2)*WeaponScale,WeaponCenterY - 8.f*ResScaleSmall - (wfHUD.WeaponInfoFrame.VL*1.5f*ResScaleSmall)*WeaponScale,Alpha, bHighlight, WeaponScale);
	}
	else
	{
		Alpha = 255 * AlphaPct * 0.3f;
		wfHUD.DrawWeaponInfo(PC,WP,None,WeaponCenterX - (wfHUD.WeaponInfoFrame.UL*ResScaleSmall/2)*WeaponScale,WeaponCenterY - 8.f*ResScaleSmall - (wfHUD.WeaponInfoFrame.VL*1.5f*ResScaleSmall)*WeaponScale,Alpha, false, WeaponScale, false, true);
	}
	Weap = GearWeapon(GetInventoryInSlot(EASlot_Holster));
	if (Weap != None)
	{
		bHighlight = (HighlightedWeaponSlot == EASlot_Holster) ? TRUE : FALSE;
		Alpha = 255 * AlphaPct * (Weap.HasAnyAmmo() ? 1.f : 0.5f);
		wfHUD.DrawWeaponInfo(PC,WP,Weap,WeaponCenterX - (wfHUD.WeaponInfoFrame.UL*ResScaleSmall/2)*WeaponScale,WeaponCenterY + 8.f*ResScaleSmall + (wfHUD.WeaponInfoFrame.VL*ResScaleSmall/2)*WeaponScale,Alpha, bHighlight, WeaponScale);
	}
	else
	{
		Alpha = 255 * AlphaPct * 0.3f;
		wfHUD.DrawWeaponInfo(PC,WP,None,WeaponCenterX - (wfHUD.WeaponInfoFrame.UL*ResScaleSmall/2)*WeaponScale,WeaponCenterY + 8.f*ResScaleSmall + (wfHUD.WeaponInfoFrame.VL*ResScaleSmall/2)*WeaponScale,Alpha, false, WeaponScale, false, true);
	}
	Weap = GearWeapon(GetInventoryInSlot(EASlot_LeftShoulder));
	if (Weap != None)
	{
		bHighlight = (HighlightedWeaponSlot == EASlot_LeftShoulder) ? TRUE : FALSE;
		Alpha = 255 * AlphaPct * (Weap.HasAnyAmmo() ? 1.f : 0.5f);
		wfHUD.DrawWeaponInfo(PC,WP,Weap,WeaponCenterX - (wfHUD.WeaponInfoFrame.UL*ResScaleSmall*1.25f)*WeaponScale,WeaponCenterY - (wfHUD.WeaponInfoFrame.VL*ResScaleSmall/2)*WeaponScale,Alpha, bHighlight, WeaponScale);
	}
	else
	{
		Alpha = 255 * AlphaPct * 0.3f;
		wfHUD.DrawWeaponInfo(PC,WP,None,WeaponCenterX - (wfHUD.WeaponInfoFrame.UL*ResScaleSmall*1.25f)*WeaponScale,WeaponCenterY - (wfHUD.WeaponInfoFrame.VL*ResScaleSmall/2)*WeaponScale,Alpha, false, WeaponScale, false, true);
	}
	Weap = GearWeapon(GetInventoryInSlot(EASlot_RightShoulder));
	if (Weap != None)
	{
		bHighlight = (HighlightedWeaponSlot == EASlot_RightShoulder) ? TRUE : FALSE;
		Alpha = 255 * AlphaPct * (Weap.HasAnyAmmo() ? 1.f : 0.5f);
		wfHUD.DrawWeaponInfo(PC,WP,Weap,WeaponCenterX + (wfHUD.WeaponInfoFrame.UL*ResScaleSmall*0.25)*WeaponScale,WeaponCenterY - (wfHUD.WeaponInfoFrame.VL*ResScaleSmall/2)*WeaponScale,Alpha, bHighlight, WeaponScale);
	}
	else
	{
		Alpha = 255 * AlphaPct * 0.3f;
		wfHUD.DrawWeaponInfo(PC,WP,None,WeaponCenterX + (wfHUD.WeaponInfoFrame.UL*ResScaleSmall*0.25)*WeaponScale,WeaponCenterY - (wfHUD.WeaponInfoFrame.VL*ResScaleSmall/2)*WeaponScale,Alpha, false, WeaponScale, false, true);
	}
}

/**
 * Calculate the center point of where the 4 weapon indicators should draw.
 */
simulated function CalculateWeaponSlotCenter( out float WeaponCenterX, out float WeaponCenterY, GearHUD_Base wfHUD, float WeaponScale )
{
	local GameViewportClient VPClient;
	local int LocalPlayerIndex;

	VPClient = LocalPlayer(wfHUD.PlayerOwner.Player).ViewportClient;
	WeaponCenterX = wfHUD.SafeZoneFriendlyCenterX;
	WeaponCenterY = wfHUD.SafeZoneFriendlyCenterY;
	LocalPlayerIndex = VPClient.ConvertLocalPlayerToGamePlayerIndex( LocalPlayer(wfHUD.PlayerOwner.Player) );

	if ( LocalPlayerIndex != -1 )
	{
		switch ( VPClient.GetSplitscreenConfiguration() )
		{
			case eSST_2P_HORIZONTAL:
			case eSST_3P_FAVOR_TOP:
			case eSST_3P_FAVOR_BOTTOM:
			case eSST_4P:
				WeaponCenterX = wfHUD.SafeZoneFriendlyCenterX + wfHUD.WeaponInfoFrame.UL*1.25f*WeaponScale;
				WeaponCenterY = wfHUD.SafeZoneFriendlyCenterY + wfHUD.WeaponInfoFrame.VL*WeaponScale;
				break;

			case eSST_2P_VERTICAL:
				if ( LocalPlayerIndex == 0 )
				{
					WeaponCenterX = wfHUD.Canvas.ClipX - wfHUD.WeaponInfoFrame.UL*1.25f*WeaponScale - 5;
				}
				else
				{
					WeaponCenterX = wfHUD.WeaponInfoFrame.UL*1.25f*WeaponScale + 5;
				}
				WeaponCenterY = wfHUD.SafeZoneFriendlyCenterY + wfHUD.WeaponInfoFrame.VL*WeaponScale;
				break;
		}
	}
}

/**
 * Updates which weapon slot is the "highlighted" one.
 */
simulated function SetHighlightedWeaponSlot( GearPawn.EAttachSlot Slot )
{
	local GearHUD_Base wfHUD;

	if (Slot != HighlightedWeaponSlot && GetInventoryInSlot(Slot) != none)
	{
		wfHUD = GearHUD_Base(TheHUD);
		if( wfHUD != None )
		{
			wfHUD.ClearWeaponSelectData();
		}

		if( (!GearPawn(Owner).bQuietWeaponEquipping)
			&& (!GearPC(GearPawn(Owner).Controller).bCinematicMode)
			&& ((WorldInfo.Game == None) || !WorldInfo.Game.bWaitingToStartMatch)
			)
		{
			//`log( "highlight bQuietWeaponEquipping: " $ GearPawn(Owner).bQuietWeaponEquipping );
			PlaySound(SoundCue'Interface_Audio.Interface.WeaponSelectHighlightCue', true);
		}

		HighlightedWeaponSlot = Slot;
	}
}


/**
 * Find inventory that matches a given slot.
 * Network: Local Player and Server
 */
simulated function Inventory GetInventoryInSlot( GearPawn.EAttachSlot Slot )
{
	local Inventory	Inv;
	local GearWeapon Weap;

	ForEach InventoryActors( class'Inventory', Inv )
	{
		Weap = GearWeapon(Inv);
		if ( (Weap != None) && (Weap.CharacterSlot == Slot) )
		{
			return Inv;
		}
	}
	return None;
}

/**
 * Sets weapon from a specified inventory slot, as the active one.
 * Network: Local Player
 */
simulated function SetWeaponFromSlot( GearPawn.EAttachSlot Slot )
{
	local GearWeapon	Weap;

	Weap = GearWeapon(GetInventoryInSlot(Slot));
	if( Weap != None )
	{
		// check to see if we have any ammo in the slot we are about to switch to
		// @note WorldInfo.Game == None on clients.
		if( WorldInfo.Game != None && !Weap.HasAnyAmmo() && !Weap.HasInfiniteSpareAmmo() )
		{
			Weap.PlayNeedsAmmoChatter();
		}

		// bring up the weapon
		if( Weap.HasAnyAmmo() || Weap.bCanSelectWithoutAmmo )
		{
			SetCurrentWeapon(Weap);
		}
	}
}

/** Switch equipped weapon to a one-hander. */
simulated function SwitchToOneHandedWeapon()
{
	local GearWeapon GW;
	local GearWeapon BestOneHandedWeapon;

	// selection criteria: pistols are preferable to grenades
	ForEach InventoryActors(class'GearWeapon', GW)
	{
		if (GW.bCanEquipWithShield)
		{
			if (BestOneHandedWeapon == None)
			{
				BestOneHandedWeapon = GW;
			}
			else if (GearWeap_GrenadeBase(BestOneHandedWeapon) != None)
			{
				if ( (GearWeap_PistolBase(GW) != None) && GW.HasAnyAmmo() )
				{
					BestOneHandedWeapon = GW;
				}
			}
		}
	}

	if (BestOneHandedWeapon != None)
	{
		SetCurrentWeapon(BestOneHandedWeapon);
	}
}

simulated final function GearPawn.EAttachSlot GetSlotFromType(class<GearWeapon> WeapClass)
{
	if (WeapClass != None)
	{
		switch( WeapClass.default.WeaponType )
		{
		case WT_Item :
			return EASlot_Belt;

		case WT_Holster :
			return EASlot_Holster;

		case WT_Normal :
			if( GetInventoryInSlot(EASlot_RightShoulder) == None )
			{
				return EASlot_RightShoulder;
			}
			return EASlot_LeftShoulder;
		}
	}
	return EASlot_None;
}


/**
 * Find available attachment slot for weapon in inventory
 */
simulated final function GearPawn.EAttachSlot FindFreeSlotForInventoryClass( class<Inventory> InvClass )
{
	local class<GearWeapon> WeapClass;

	WeapClass = class<GearWeapon>(InvClass);
	if (WeapClass != None)
	{
		switch( WeapClass.default.WeaponType )
		{
		case WT_Item :
			if( GetInventoryInSlot(EASlot_Belt) == None )
			{
				return EASlot_Belt;
			}
			break;

		case WT_Holster :
			if( GetInventoryInSlot(EASlot_Holster) == None )
			{
				return EASlot_Holster;
			}
			break;

		case WT_Normal :
			if( GetInventoryInSlot(EASlot_RightShoulder) == None )
			{
				return EASlot_RightShoulder;
			}
			else if( GetInventoryInSlot(EASlot_LeftShoulder) == None )
			{
				return EASlot_LeftShoulder;
			}
			break;
		}
	}


	return EASlot_None;
}


/**
 * If Server in a network game, then replicate CurrentWeapon changes.
 * Do this only when we actually switch weapons
 * As on local player or server, setting PendingWeapon is no guarantee that the weapon will switch right away.
 * So we replicate changes only when the player actually switches to a new weapon.
 */
simulated function ReplicateCurrentWeaponToRemoteClients(Weapon CurrentWeapon, optional bool bHadAPreviousWeapon)
{
	local GearPawn			P;
	local class<GearWeapon>	CurrentWeaponClass;

	P = GearPawn(Owner);
	if( P != None && P.Role == ROLE_Authority )
	{
		if( CurrentWeapon != None )
		{
			CurrentWeaponClass = class<GearWeapon>(CurrentWeapon.class);
		}

		if( P.RemoteWeaponRepInfo.WeaponClass != CurrentWeaponClass )
		{
			// Set up weapon slave replication to remote clients.
			P.RemoteWeaponRepInfo.WeaponClass				= CurrentWeaponClass;
			P.RemoteWeaponRepInfo.BYTE_bHadAPreviousWeapon	= BYTE(bHadAPreviousWeapon);
			P.bForceNetUpdate = TRUE;

			`LogInv("Weapon Class set for replication to remote clients." @ CurrentWeaponClass @ "bHadAPreviousWeapon:" @ bHadAPreviousWeapon);
		}
	}
}


/**
 * Notification called from GearWeapon when the current weapon is being put down.
 */
simulated function PuttingWeaponDown()
{
	local GearPawn P;

	P = GearPawn(Owner);
	// If we're putting our weapon down and we have a PendingWeapon, then replicate this
	// So remote clients can start switching as well.
	// We anticipate the switch, so there is no delay on remote clients.
	// If the switch didn't go as planned, we fix this in ChangedWeapon().
	if( PendingWeapon != None )
	{
		ReplicateCurrentWeaponToRemoteClients(PendingWeapon, (P != None && P.MyGearWeapon != None));
	}
}

/** Player has equipped a new weapon, make sure all clients are synched */
simulated function WeaponEquipped()
{
	if( Instigator.Weapon != None )
	{
		ReplicateCurrentWeaponToRemoteClients(Instigator.Weapon, FALSE);
	}
}

/** @see InventoryManager::ServerChangedWeapon */
simulated function ChangedWeapon()
{
	local GearWeapon				CurrentWeapon;
	local GearPawn.EAttachSlot	DesiredSlot;
	local Weapon				OldWeapon;
	local GearPlayerCamera		GPCam;
	local GearPC				PC;
	local GearPawn				P;

	P	= GearPawn(Owner);
	if(P != none)
	{
		PC	= GearPC(P.Controller);
	}


	// indicate we should draw the weapon indicator
	if( P != None && PC != None )
	{
		PC.SetLastWeaponInfoTime(WorldInfo.TimeSeconds);
	}

	// Save current weapon as old weapon
	OldWeapon = Instigator.Weapon;
	if (GearWeap_HeavyBase(OldWeapon) == None)
	{
		// only save if not a heavy, since heavy's get tossed
		PreviousEquippedWeapon = GearWeapon(OldWeapon);
	}

	// Equip PendingWeapon
	Super.ChangedWeapon();

	// We have now equipped a new weapon so replicate this change to remote clients, to make sure they're in synch
	ReplicateCurrentWeaponToRemoteClients(Instigator.Weapon, (OldWeapon != None));

	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "Instigator:" @ Instigator @ "Weapon:" @ Instigator.Weapon @ "OldWeapon:" @ OldWeapon);

	if( Role == ROLE_Authority && P != None )
	{
		// if we're switching to a weapon that wasn't assigned a slot, fix it up
		CurrentWeapon = GearWeapon(P.Weapon);
		if( CurrentWeapon != None &&
			CurrentWeapon != OldWeapon &&
			CurrentWeapon.CharacterSlot == EASlot_None )
		{
			//`log( GetFuncName() @ "weapon had no slot, fix it up!!" $ " Pawn: " $ Pawn(Owner) $ " CurrentWeapon: " $ CurrentWeapon $ " OldWeapon: " $ OldWeapon );
			DesiredSlot = FindFreeSlotForInventoryClass(CurrentWeapon.class);
			CurrentWeapon.AssignToSlot(DesiredSlot);
		}
	}

	if( PC != None )
	{
		GPCam = GearPlayerCamera(PC.PlayerCamera);
		if ( (GPCam != None) && (GPCam.CurrentCamera == GPCam.GameplayCam) )
		{
			GPCam.GameplayCam.CurrentCamMode.WeaponChanged(PC, OldWeapon, Instigator.Weapon);
		}
	}
}


simulated function SetPendingWeapon(Weapon DesiredWeapon)
{
	local GearWeapon DesiredGW;

	DesiredGW = GearWeapon(DesiredWeapon);
	if ( (Shield != None) && !DesiredGW.bCanEquipWithShield )
	{
		`LogInv("Dropping shield!");
		// can't hold shield with non-1-handed weapon, so drop it
		GearPawn(Instigator).ServerDropShield();
	}
	super.SetPendingWeapon(DesiredWeapon);
}

/**
 * Tries to find weapon of class ClassName, and switches to it
 *
 * @param	ClassName, class of weapon to switch to
 */
exec function SwitchToWeaponClass( String ClassName, optional bool bCreate )
{
	local Weapon		CandidateWeapon;
	local class<Weapon>	WeapClass;

	WeapClass = class<Weapon>(DynamicLoadObject(ClassName, class'Class'));

	if ( WeapClass != None )
	{
		CandidateWeapon = Weapon(FindInventoryType( WeapClass ));
		if (bCreate && CandidateWeapon == None)
		{
			CandidateWeapon = Weapon(CreateInventory(WeapClass,TRUE));
		}
		if ( CandidateWeapon != None )
		{
			SetCurrentWeapon( CandidateWeapon );
		}
		else
		{
			`log("SwitchToWeaponClass weapon not found in inventory" @ String(WeapClass) );
		}
	}
	else
	{
		`log("SwitchToWeaponClass weapon class not found" @ ClassName );
	}
}

/**
 * This will find the weapon which can use this ammo type and return it;
 *
 * returns none if there are no weapons that can use this ammo type
 */
simulated function GearWeapon FindWeaponThatCanUseThisAmmoType(class<GearAmmoType> AmmoTypeClass)
{
	local GearWeapon Weap;

	ForEach InventoryActors(class'GearWeapon', Weap)
	{
		if( Weap.AmmoTypeClass == AmmoTypeClass )
		{
			return Weap;
		}
	}

	return None;
}


/** Find a carried weapon of weapon type WeaponType */
simulated function GearWeapon FindWeaponOfType(GearWeapon.EWeaponType WeaponType)
{
	local GearWeapon Weap;

	ForEach InventoryActors(class'GearWeapon', Weap)
	{
		if( Weap.WeaponType == WeaponType )
		{
			return Weap;
		}
	}

	return None;
}


/**
 * Returns TRUE if Pawn P can pick up ammo of this AmmoType
 * bExcludeChecks - TRUE = could you have?
 *                - FALSE = you actually can
 */
simulated function bool CanPickUpAmmoType(class<GearAmmoType> AmmoTypeClass, bool bExcludeChecks)
{
	local GearWeapon	Weap;

	// See if we can find this weapon in our inventory
	Weap = FindWeaponThatCanUseThisAmmoType(AmmoTypeClass);
	if( Weap != None && (bExcludeChecks || Weap.CanPickupAmmo()) )
	{
		return TRUE;
	}

	return FALSE;
}


/**
 * Finds out if player is carrying weapon, and adds ammunition if so.
 * Returns TRUE if successful
 */
function bool AddAmmoFromAmmoType(class<GearAmmoType> AmmoTypeClass, float NumMagazines)
{
	local GearWeapon	Weap;
	local int AmmoAmount;

	Weap = FindWeaponThatCanUseThisAmmoType(AmmoTypeClass);
	if( Weap != None &&
		Weap.GetMaxSpareAmmoSize() > 0 &&
		Weap.SpareAmmoCount < Weap.GetMaxSpareAmmoSize() )
	{
		AmmoAmount = Weap.AddAmmo(Weap.GetMagazineSize() * NumMagazines);
		Weap.AddAmmoMessage( AmmoAmount );
		return TRUE;
	}

	return FALSE;
}

/**
 * Finds out if player is carrying weapon, and adds ammunition if so.
 * Returns TRUE if successful
 */
function bool AddAmmoFromWeaponClass(class<GearWeapon> WeaponClass, int SpecifiedAmount)
{
	local GearWeapon	Weap;
	local int AmmoAmount;

	Weap = GearWeapon(FindInventoryType(WeaponClass, TRUE));
	if( Weap != None &&
		Weap.CanPickupAmmo() )
	{
		if( SpecifiedAmount == 0 )
		{
			SpecifiedAmount = Weap.GetDefaultAmmoAmount();
		}

		AmmoAmount = Weap.AddAmmo(SpecifiedAmount);
		Weap.AddAmmoMessage( AmmoAmount );
		return TRUE;
	}

	return FALSE;
}

/**
 * Returns TRUE is weapon class can be swapped with currently held weapon.
 */
simulated function bool CanSwapWeaponClassWithCurrent(class<GearWeapon> WeaponClass)
{
	// if the weapon is swappable and if weapons are the same type, then no problem
	if( GearWeapon(Instigator.Weapon).bSwappable && GearWeapon(Instigator.Weapon).WeaponType == WeaponClass.default.WeaponType )
	{
		return TRUE;
	}

	return FALSE;
}


/** Make sure Player doesn't carry more than 4 weapons! */
simulated function Inventory CreateInventory(class<Inventory> NewInventoryItemClass, optional bool bDoNotActivate)
{
	local int			WeapCount;
	local int			HeavyWeapCount;
	local GearWeapon	Weap;
	local bool			bCreatingHeavyWeap;

	local bool			bDoNotCreate;
	local Inventory		NewInvItem;

	local GearAI		AI;

	// if this is an idea give them a chance to reject it (creatures, etc)
	AI = GearAI(Pawn(Owner).Controller);
	if (AI != None &&
		!AI.AllowWeaponInInventory(class<GearWeapon>(NewInventoryItemClass)))
	{
		return None;
	}


	// enforce max weapons limits
	if (!bDebugIgnoreWeaponCountLimit)
	{
		// Disable max 4 inventory check for remote clients. As these will carry in their inventory all weapons they've ever switched to.
		if( WorldInfo.NetMode != NM_Client || Instigator.IsLocallyControlled() )
		{
			if (ClassIsChildOf(NewInventoryItemClass, class'GearWeapon'))
			{
				ForEach InventoryActors(class'GearWeapon', Weap)
				{
					if (Weap.WeaponType == WT_Heavy)
					{
						HeavyWeapCount++;
					}
					else
					{
						WeapCount++;
					}
				}

				// can have 4 non-heavy weapons and 1 heavy weapon
				bCreatingHeavyWeap = ClassIsChildOf(NewInventoryItemClass, class'GearWeap_HeavyBase');
				if (bCreatingHeavyWeap)
				{
					if (HeavyWeapCount >= 1)
					{
						bDoNotCreate = TRUE;
`if(`notdefined(FINAL_RELEASE))
						`log(Instigator @ "already has a heavy weapon!!!!! Aborting Creation of" @ NewInventoryItemClass);
						ScriptTrace();
						`log("Likely coming from USeqAct_AIFactory::UpdateSpawnSets, Set.LoadoutClasses[] ?");
`endif
					}
				}
				else
				{
					if (WeapCount >= 4)
					{
						bDoNotCreate = TRUE;
`if(`notdefined(FINAL_RELEASE))
						`log(Instigator @ "already has 4 weapons!!!!! Aborting Creation of" @ NewInventoryItemClass);
						ScriptTrace();
						`log("Likely coming from USeqAct_AIFactory::UpdateSpawnSets, Set.LoadoutClasses[] ?");
`endif
					}
				}
			}
		}
	}


	if (!bDoNotCreate)
	{
		// create it
		NewInvItem = Super.CreateInventory(NewInventoryItemClass, bDoNotActivate);
	}


	return NewInvItem;
}

simulated function bool AddInventory(Inventory NewItem, optional bool bDoNotActivate)
{
	if (Super.AddInventory(NewItem, bDoNotActivate))
	{
		if (NewItem.IsA('GearShield'))
		{
			Shield = GearShield(NewItem);
			`LogInv("Shield set to" @ Shield);
		}
		return true;
	}
	else
	{
		return false;
	}
}

simulated function DiscardClientSideInventory()
{
	local Inventory	Inv;

	if( WorldInfo.NetMode == NM_Client && !bDeleteMe && Instigator != None)
	{
		`LogInv("");

		ForEach InventoryActors(class'Inventory', Inv)
		{
			if( Inv.Role == ROLE_Authority )
			{
				Inv.Destroy();
			}
		}

		// Clear reference to Weapon
		Instigator.Weapon = None;

		// Clear reference to PendingWeapon
		PendingWeapon = None;

		Shield = None;

		Instigator.InvManager = None;

		Instigator = None;

		// destroy locally since this is most likely from a map change and the inv manager is about to be nuked
		// and the client will recreate if needed ttp 113128
		Destroy();
	}
}

/**
 * Overridden to check for GearPawn.bAllowInventoryDrops.
 */
simulated function DiscardInventory()
{
	local Inventory	Inv;
	local vector	TossVelocity;
	local GearPawn GP;

	`LogInv("");

	//`log(`location@`showobj(Owner));
//	scripttrace();

	DiscardClientSideInventory();

	GP = GearPawn(Instigator);
	ForEach InventoryActors(class'Inventory', Inv)
	{
		if( (GP == None || GP.bAllowInventoryDrops) && Inv.bDropOnDeath && Instigator != None )
		{
			TossVelocity = vector(Instigator.GetViewRotation());
			TossVelocity = TossVelocity * ((Instigator.Velocity dot TossVelocity) + 500.f) + 250.f * VRand() + vect(0,0,250);
			Inv.DropFrom(Instigator.Location, TossVelocity);
		}
		else
		{
			Inv.Destroy();
		}
	}

	// Clear reference to Weapon
	Instigator.Weapon = None;

	// Clear reference to PendingWeapon
	PendingWeapon = None;

	Shield = None;
}


simulated function float GetWeaponRatingFor(Weapon W)
{
	local GearWeapon GearWeap;

	if (Role < ROLE_Authority && Instigator.Controller == None)
	{
		`Warn("Called on" @ W @ "with Instigator" @ Instigator @ "but no Instigator.Controller!");
	}

	// for human players, prefer weapons based on slot so that autoselect on spawn, etc. is consistent
	GearWeap = GearWeapon(W);
	if (GearWeap != None && Instigator.IsHumanControlled())
	{
		if (!GearWeap.HasAnyAmmo())
		{
			return -1;
		}
		else
		{
			if (GearWeap.WeaponType == WT_Heavy)
			{
				return 5.0;
			}
			switch (GearWeap.CharacterSlot)
			{
				case EASlot_RightShoulder:
					return 4.0;
				case EASlot_LeftShoulder:
					return 3.0;
				case EASlot_Holster:
					return 2.0;
				default:
					return 1.0;
			}
		}
	}
	else
	{
		//debug
		`AILog_Ext( self@GetFuncName()@W@Super.GetWeaponRatingFor(W), 'Weapon', GearAI(Instigator.Controller));

		return Super.GetWeaponRatingFor(W);
	}
}

reliable client function ClientRemoveShieldFromInventory()
{
	Shield = None;
}

simulated function RemoveFromInventory(Inventory ItemToRemove)
{
	local GearPawn P;

	P = GearPawn(Owner);

	if( ItemToRemove == Shield )
	{
		Shield = None;
		ClientRemoveShieldFromInventory();
	}

	// If getting rid of our weapon, then clear reference to it.
	if( P != None && P.MyGearWeapon == ItemToRemove )
	{
		P.MyGearWeapon = None;
		P.Weapon = None;
		// Replicate the weapon thrown, so it clears on clients as well.
		ReplicateCurrentWeaponToRemoteClients(None, FALSE);
	}

	Super.RemoveFromInventory(ItemToRemove);
}

simulated function Inventory FindInventoryByClass(Name ClassName)
{
	local Inventory		Inv;

	ForEach InventoryActors(class'Inventory', Inv)
	{
		if ( Inv.IsA(ClassName) )
		{
			return Inv;
		}
	}

	return None;
}

simulated function ClientWeaponSet(Weapon NewWeapon, bool bOptionalSet, optional bool bDoNotActivate)
{
	if (WorldInfo.NetMode != NM_Client || (Instigator != None && Instigator.IsLocallyControlled()))
	{
		Super.ClientWeaponSet(NewWeapon, bOptionalSet, bDoNotActivate);
	}

	// Make sure weapon doesn't get stuck in PendingClientWeaponSet state.
	if( NewWeapon.IsInState('PendingClientWeaponSet') )
	{
		NewWeapon.GotoState('Inactive');
	}
}

defaultproperties
{
	WeapSelHUDTimeOut=1
	PendingFire(0)=0
	PendingFire(1)=0
	PendingFire(2)=0
	PendingFire(3)=0
	PendingFire(4)=0
	PendingFire(5)=0
	WeapSelHUDTimeCount=-1
	WeapSelHUDTimeCountSpeedMult=1.0f

	LinearSlotOrder(0)=EASlot_RightShoulder
	LinearSlotOrder(1)=EASlot_LeftShoulder
	LinearSlotOrder(2)=EASlot_Holster
	LinearSlotOrder(3)=EASlot_Belt

	InventoryTossPitch=-4000
	InventoryTossSpeed=400
}
