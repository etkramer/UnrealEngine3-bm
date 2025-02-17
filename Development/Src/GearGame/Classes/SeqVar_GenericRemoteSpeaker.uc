/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqVar_GenericRemoteSpeaker extends SeqVar_Object
	native(Sequence);

cpptext
{
	UObject** GetObjectRef( INT Idx );

	virtual FString GetValueStr()
	{
		return FString(TEXT("Generic Speaker (Remote)"));
	}
};

defaultproperties
{
	ObjName="Generic Speaker"
	ObjCategory="Object"
	SupportedClasses.Empty
}
