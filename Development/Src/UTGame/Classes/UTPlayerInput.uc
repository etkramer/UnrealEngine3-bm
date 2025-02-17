/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTPlayerInput extends GamePlayerInput within UTPlayerController
	native;

var float LastDuckTime;
var bool  bHoldDuck;
var Actor.EDoubleClickDir ForcedDoubleClick;

cpptext
{
	virtual UBOOL InputKey(INT ControllerId, FName Key, enum EInputEvent Event, FLOAT AmountDepressed = 1.f, UBOOL bGamepad = FALSE);
}

/**
 * Clears any pending inputs
 */
native function ForceFlushOfInput();

simulated exec function Duck()
{
	if ( UTPawn(Pawn)!= none )
	{
		if (bHoldDuck)
		{
			bHoldDuck=false;
			bDuck=0;
			return;
		}

		bDuck=1;

		if ( WorldInfo.TimeSeconds - LastDuckTime < DoubleClickTime )
		{
			bHoldDuck = true;
		}

		LastDuckTime = WorldInfo.TimeSeconds;
	}
}

simulated exec function UnDuck()
{
	if (!bHoldDuck)
	{
		bDuck=0;
	}
}

exec function Jump()
{
	local UTPawn P;

	if (!IsMoveInputIgnored())
	{
		// jump cancels feign death
		P = UTPawn(Pawn);
		if (P != None && P.bFeigningDeath)
		{
			P.FeignDeath();
		}
		else
		{
		 	if (bDuck>0)
		 	{
		 		bDuck = 0;
		 		bHoldDuck = false;
		 	}
			Super.Jump();
		}
	}
}

event PlayerInput( float DeltaTime )
{
	local vector Dir;
	local float Dist, Ang, Leeway, OffAmount;
	local int Quad, CurrentSel, PrevItem, NextItem, CurrentWeaponIndex;
	local UTHud Hud;
	local array<QuickPickCell> Cells;

	HUD = UTHud(MyHud);
	if ( Hud != none && Hud.bShowQuickPick && (Hud.QuickPickDeltaAngle != 0.0) )
	{
		//@warning: assumption of input scale in DefaultInput.ini
		Dir.X = aTurn;
		Dir.Y = aLookup / 0.75;
		// skew the results a little as the input doesn't entirely give us a normal
		// particularly when pressing near a cardinal direction (could get e.g. X=1.0,Y=0.3)
		if (Dir.X >= 0.99)
		{
			Dir.X -= Abs(Dir.Y);
		}
		else if (Dir.X <= -0.99)
		{
			Dir.X += Abs(Dir.Y);
		}
		else if (Dir.Y >= 0.99)
		{
			Dir.Y -= Abs(Dir.X);
		}
		else if (Dir.Y <= -0.99)
		{
			Dir.Y += Abs(Dir.X);
		}

		Dist = VSize(Dir);
		if (Dist > 0.6)
		{
			Ang = (static.GetHeadingAngle(Normal(Dir)) * 57.2957795) + 90;
			Ang += Hud.QuickPickDeltaAngle * 0.5;

			if (Ang < 0)
			{
				Ang = 360 + Ang;
			}

			Quad = int(Abs(Ang) / Hud.QuickPickDeltaAngle);
			// give some more leeway towards retaining current selection
			CurrentSel = Hud.QuickPickCurrentSelection;
			PrevItem = (Quad == 0) ? (Hud.QuickPickNumCells - 1) : (Quad - 1);
			NextItem = (Quad + 1) % Hud.QuickPickNumCells;
			Leeway = Hud.QuickPickDeltaAngle * 0.20;
			OffAmount = Abs(Ang) % Hud.QuickPickDeltaAngle;
			if ( Hud.bQuickPickMadeNewSelection && CurrentSel != -1 && Quad != CurrentSel &&
				(CurrentSel == NextItem || CurrentSel == PrevItem) )
			{
				if (OffAmount < Leeway || OffAmount > Hud.QuickPickDeltaAngle - Leeway)
				{
					Quad = Hud.QuickPickCurrentSelection;
				}
			}
			else
			{
				if (UTPawn(Pawn) != None)
				{
					UTPawn(Pawn).GetQuickPickCells(Hud, Cells, CurrentWeaponIndex);
				}
				else if (UTVehicleBase(Pawn) != None)
				{
					UTVehicleBase(Pawn).GetQuickPickCells(Hud, Cells, CurrentWeaponIndex);
				}
				if (Cells.length > 0 && Cells[Quad].Icon == None)
				{
					if (Cells[PrevItem].Icon != None && OffAmount < Leeway)
					{
						Quad = PrevItem;
					}
					else if (Cells[NextItem].Icon != None && OffAmount > Hud.QuickPickDeltaAngle - Leeway)
					{
						Quad = NextItem;
					}
				}
			}

			Hud.QuickPick(Quad);
		}

		aLookup = 0.0;
		aTurn = 0.0;
	}

	Super.PlayerInput(Deltatime);
}

/** Will return the BindName based on the BindCommand */
native function String GetUTBindNameFromCommand( String BindCommand );

defaultproperties
{
	bEnableFOVScaling=true
}
