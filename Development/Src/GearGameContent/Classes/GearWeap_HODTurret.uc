/**
 * Troika Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_HODTurret extends GearWeapon
	deprecated;

var() transient HOD_TurretBeam		ActiveBeam;

simulated function AlignBeam()
{
	local ImpactInfo WeaponHit;
	local vector StartTrace, EndTrace;

	if (ActiveBeam != None)
	{
		StartTrace = GetMuzzleLoc();
		//DrawDebugBox(StartTrace, vect(6,6,6), 255, 255, 0);
		EndTrace = StartTrace + vector(GetAdjustedAim(StartTrace)) * GetTraceRange();
		WeaponHit = CalcWeaponFire(StartTrace, EndTrace);

		ActiveBeam.SetEndpoints(StartTrace, WeaponHit.HitLocation);
	}
}

simulated function TurnOnHODBeam()
{
	if(ActiveBeam == None)
	{
		// spawn HOD_TurretBeam  
		ActiveBeam = Spawn(class'HOD_TurretBeam');

		if (ActiveBeam != None)
		{
			ActiveBeam.Init(Instigator);
			ActiveBeam.BeginWarmup(vect(0,0,0), FALSE);			// params don't matter
			AlignBeam();
		}	
	}
}

simulated function TurnOffHODBeam()
{
	if (ActiveBeam != None)
	{
		ActiveBeam.AbortFire();
		ActiveBeam = None;
	}
}

simulated function Tick(float DeltaTime)
{
	AlignBeam();
	super.Tick(DeltaTime);
}

simulated function BeginFire(Byte FireModeNum)
{
	if (FireModeNum == 0)
	{
		TurnOnHODBeam();
	}
}


simulated function EndFire(Byte FireModeNum)
{
	TurnOffHODBeam();
}

defaultproperties
{
	DamageTypeClassForUI=class'GDT_HODTurret'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=146,V=96,UL=143,VL=46)
	HUDDrawData=(DisplayCount=106,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=494,UL=106,VL=7),ULPerAmmo=0)
}