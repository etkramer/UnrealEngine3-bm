/**
 * This sequence variable type allows designers to manipulate byte values.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class SeqVar_Byte extends SequenceVariable
	native(inherit);

var()	byte	ByteValue;

cpptext
{
	virtual BYTE*	GetByteRef();
	virtual FString	GetValueStr();
	virtual UBOOL	SupportsProperty(UProperty *Property);

	/**
	 * Copies the value stored by this SequenceVariable to the SequenceOp member variable that it's associated with.
	 *
	 * @param	Op			the sequence op that contains the value that should be copied from this sequence variable
	 * @param	Property	the property in Op that will receive the value of this sequence variable
	 * @param	VarLink		the variable link in Op that this sequence variable is linked to
	 */
	virtual void PublishValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);

	/**
	 * Copy the value from the member variable this VariableLink is associated with to this VariableLink's value.
	 *
	 * @param	Op			the sequence op that contains the value that should be copied to this sequence variable
	 * @param	Property	the property in Op that contains the value to copy into this sequence variable
	 * @param	VarLink		the variable link in Op that this sequence variable is linked to
	 */
	virtual void PopulateValue(USequenceOp *Op, UProperty *Property, FSeqVarLink &VarLink);
}

DefaultProperties
{
	ObjName="Byte"
	ObjCategory="Byte"
	ObjColor=(R=128,G=64,B=64,A=255)		// maroon / rust
}
