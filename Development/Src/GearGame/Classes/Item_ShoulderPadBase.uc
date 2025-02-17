/**
 * NOTE:  for the AttachSocketName we use the base part of the name which does not include the 'left' or 'right' suffix
 * (e.g. armorupperarm instead of armorupperarmleft)  Then in the function to attach to the specifc arm we append 'left' or 'right'
 * So: this means content need to make their shoulderpads work on either left or right of the mesh
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Item_ShoulderPadBase extends ItemSpawnable
	abstract;



defaultproperties
{
	ImpactingGround=SoundCue'Foley_BodyMoves.BodyMoves.Helmet_ImpactCue'
}
