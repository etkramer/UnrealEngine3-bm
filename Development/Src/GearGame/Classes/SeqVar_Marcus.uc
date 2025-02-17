/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqVar_Marcus extends SeqVar_Object
	native(Sequence);

cpptext
{
	UObject** GetObjectRef( INT Idx );

	virtual FString GetValueStr()
	{
		return FString(TEXT("Marcus"));
	}

	virtual UBOOL SupportsProperty(UProperty *Property)
	{
		return FALSE;
	}
}

defaultproperties
{
	ObjName="Marcus"
	ObjCategory="Player"
}
