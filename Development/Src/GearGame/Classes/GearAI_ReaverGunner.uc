/**
 * Class to handle the AI controlling of the Reaver
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_ReaverGunner extends GearAI
		config(AI);


function bool NotifyBump( Actor Other, vector HitNormal );


function bool ShouldSaveForCheckpoint()
{
	// don't save in checkpoints as this guy will get respawned when the reaver is reloaded
	return false;
}

defaultproperties
{
	DefaultCommand=class'AICmd_Base_MountedGunner'
	bCanRevive=FALSE
}
