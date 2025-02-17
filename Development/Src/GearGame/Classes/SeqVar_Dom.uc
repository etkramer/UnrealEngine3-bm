/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqVar_Dom extends SeqVar_Object
	native(Sequence);

cpptext
{
	UObject** GetObjectRef( INT Idx );

	virtual FString GetValueStr()
	{
		return FString(TEXT("Dom"));
	}

	virtual UBOOL SupportsProperty(UProperty *Property)
	{
		return FALSE;
	}
}

defaultproperties
{
	ObjName="Dom"
	ObjCategory="Player"
}
