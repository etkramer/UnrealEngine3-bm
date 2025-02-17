/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//
// OptionalObject is an Pickup class
//
class UTPickupMessage extends UTLocalMessage;

static simulated function ClientReceive( 
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1, 
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	local UTHUD HUD;

	Super.ClientReceive(P, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);

	HUD = UTHUD(P.MyHUD);
	if ( HUD != None )
	{
		if ( class<UTPickupFactory>(OptionalObject) != None )
		{
			class<UTPickupFactory>(OptionalObject).static.UpdateHUD(HUD);
		}
		else if ( class<UTWeapon>(OptionalObject) != None )
		{
			HUD.LastWeaponBarDrawnTime = HUD.WorldInfo.TimeSeconds + 2.0;
			HUD.LastPickupTime = HUD.WorldInfo.TimeSeconds;
		}
		else
		{
			HUD.LastPickupTime = HUD.WorldInfo.TimeSeconds;
		}
	}		
}

defaultproperties
{
	bIsUnique=true
	bCountInstances=true
	DrawColor=(R=255,G=255,B=128,A=255)
	FontSize=1
	bIsConsoleMessage=false
	MessageArea=5
}
