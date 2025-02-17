/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** takes a character name and converts it to the Controller of the player who is playing that character
 * always works on bots, only works on humans in the single player campaign
 */
class UTSeqVar_Character extends SeqVar_Object
	native(Sequence);

cpptext
{
	/** @return whether C is playing the character specified by our CharacterName */
	UBOOL MatchCharacter(AController* C);

	UObject** GetObjectRef(INT Idx);

	virtual FString GetValueStr()
	{
		return (CharacterName != TEXT("")) ? CharacterName : FString(TEXT("None"));
	}
};

var() string CharacterName;

defaultproperties
{
	ObjName="Character"
	ObjCategory="Object"
}
