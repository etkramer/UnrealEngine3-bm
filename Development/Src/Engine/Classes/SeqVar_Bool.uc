/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqVar_Bool extends SequenceVariable
	native(Sequence);

cpptext
{
	UBOOL* GetBoolRef()
	{
		return (UBOOL*)&bValue;
	}

	FString GetValueStr()
	{
		return bValue == TRUE ? GTrue : GFalse;
	}

	virtual UBOOL SupportsProperty(UProperty *Property)
	{
		return (Property->IsA(UBoolProperty::StaticClass()));
	}

	virtual void PublishValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
	virtual void PopulateValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
}

var() int			bValue;

// Red bool - gives you wings!
defaultproperties
{
	ObjName="Bool"
	ObjColor=(R=255,G=0,B=0,A=255)		// red
}
