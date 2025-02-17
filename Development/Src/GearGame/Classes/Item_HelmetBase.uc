/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Item_HelmetBase extends ItemSpawnable
	abstract;

var bool bCanBeShotOff;

defaultproperties
{
	// Locust_Grunt.Emissive.Geist_Grunt_EmissivehelmetRED_Mat: New Red Helmet Shader
    BeingShotOffSound=SoundCue'Foley_BodyMoves.BodyMoves.Helmet_RipOffGameyBCue' // 'Foley_BodyMoves.BodyMoves.Helmet_RipOffCue'
	ImpactingGround=SoundCue'Foley_BodyMoves.BodyMoves.Helmet_ImpactCue'
	bCanBeShotOff=TRUE
}



