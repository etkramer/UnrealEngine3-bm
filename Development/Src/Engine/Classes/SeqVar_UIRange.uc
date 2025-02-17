/**
 * This class allows designers to manipulate UIRangeData values.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqVar_UIRange extends SequenceVariable
	native(UISequence)
	DependsOn(UIRoot);

cpptext
{
	virtual struct FUIRangeData* GetUIRangeRef();
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
var()	UIRoot.UIRangeData	RangeValue;

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
	ObjName="UI Range"
	ObjCategory="UI"
	ObjColor=(R=128,G=128,B=192,A=255)	// light purple
}
