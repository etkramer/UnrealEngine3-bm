/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqVar_Anya extends SeqVar_Object
	native(Sequence);

cpptext
{
	UObject** GetObjectRef( INT Idx );

	virtual FString GetValueStr()
	{
		return FString(TEXT("Anya (Remote)"));
	}
};

defaultproperties
{
	ObjName="Anya"
	ObjCategory="Object"
	SupportedClasses.Empty
}
