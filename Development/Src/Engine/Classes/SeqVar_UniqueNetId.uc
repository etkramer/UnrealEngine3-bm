/**
 * This sequence variable type allows designers to manipulate UniqueNetId values.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class SeqVar_UniqueNetId extends SequenceVariable
	native(UISequence);

cpptext
{
	virtual struct FUniqueNetId* GetUniqueNetIdRef();
	virtual FString GetValueStr();
	virtual UBOOL SupportsProperty(UProperty *Property);

	/**
	 * Copies the value stored by this SequenceVariable to the SequenceOp member variable that it's associated with.
	 */
	virtual void PublishValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);

	/**
	 * Copy the value from the member variable this VariableLink is associated with to this VariableLink's value.
	 */
	virtual void PopulateValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
}

/**
 * The value associated with this sequence variable.
 */
var()	editconst	UniqueNetId		NetIdValue;

/**
 * Determines whether this class should be displayed in the list of available ops in the level kismet editor.
 *
 * @return	TRUE if this sequence object should be available for use in the level kismet editor
 */
event bool IsValidLevelSequenceObject()
{
	return false;
}

DefaultProperties
{
	ObjName="Unique NetID"
	ObjCategory="UI"
	ObjColor=(R=128,G=192,B=192,A=255)	// light purple
}
