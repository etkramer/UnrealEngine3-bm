/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Defines an arbitrary point for AI to emerge from
 */
class GearSpawner_EholeSpawnLocation extends Actor
	dependson(GearSpawner)
	native
	placeable;

/** Whether or not to use this spawn point. (e.g. useful for making different facing E-holes **/
var() bool bEnabled;


/** Which animation to play when emerging from the ehole **/
var() EHoleEmergeAnim EmergeAnim;



defaultproperties
{
	bEnabled=TRUE
	EmergeAnim=EHEA_Random


	Begin Object Class=SpriteComponent Name=Sprite
		Sprite=Texture2D'EditorResources.S_Actor'
		HiddenGame=TRUE
		AlwaysLoadOnClient=FALSE
		AlwaysLoadOnServer=FALSE
		bIsScreenSizeScaled=TRUE
		ScreenSize=0.0025
	End Object
	Components.Add(Sprite)

	Begin Object Class=ArrowComponent Name=ArrowComponent0
		ArrowColor=(R=0,G=255,B=128)
		ArrowSize=1.5
		bTreatAsASprite=True
		AlwaysLoadOnClient=FALSE
		AlwaysLoadOnServer=FALSE
	End Object
	Components.Add(ArrowComponent0)
}
