/**
 * SeqAct_GearUnlockableUnlock - Kismet action that allows unlockables to be unlocked
 * by events that occur in levels.  This action will attempt to unlock the unlockable
 * for all players.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GearUnlockableUnlock extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void Activated();
}

/** Enum of the achievements to be unlocked by events that occur in levels */
enum EGearLevelUnlockable
{
	eGLUNLOCK_Character_Dizzy,
	eGLUNLOCK_Character_Kantus,
	eGLUNLOCK_Character_Tai,
	eGLUNLOCK_Character_PalaceGuard,
	eGLUNLOCK_Character_Skorge,
	eGLUNLOCK_Character_DLC1,
	eGLUNLOCK_Character_DLC2,
	eGLUNLOCK_Character_DLC3,
	eGLUNLOCK_Character_DLC4,
	eGLUNLOCK_Character_DLC5,
};

/**
 * Mapping of unlockables that can be unlocked via this kismet action to the real unlockable types
 * that the unlockable system recognizes them as (EGearLevelUnlockable -> EGearUnlockable)
*/
var array<EGearUnlockable> UnlockableMap;

/** The unlockable you'd like to unlock */
var() EGearLevelUnlockable Unlockable;

defaultproperties
{
	ObjName="Unlockable Unlock"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Unlock")

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets,bHidden=TRUE)

	UnlockableMap(eGLUNLOCK_Character_Dizzy)=eUNLOCK_Character_Dizzy
	UnlockableMap(eGLUNLOCK_Character_Kantus)=eUNLOCK_Character_Kantus
	UnlockableMap(eGLUNLOCK_Character_Tai)=eUNLOCK_Character_Tai
	UnlockableMap(eGLUNLOCK_Character_PalaceGuard)=eUNLOCK_Character_PalaceGuard
	UnlockableMap(eGLUNLOCK_Character_Skorge)=eUNLOCK_Character_Skorge
	UnlockableMap(eGLUNLOCK_Character_DLC1)=eUNLOCK_Character_DLC1
	UnlockableMap(eGLUNLOCK_Character_DLC2)=eUNLOCK_Character_DLC2
	UnlockableMap(eGLUNLOCK_Character_DLC3)=eUNLOCK_Character_DLC3
	UnlockableMap(eGLUNLOCK_Character_DLC4)=eUNLOCK_Character_DLC4
	UnlockableMap(eGLUNLOCK_Character_DLC5)=eUNLOCK_Character_DLC5
}
