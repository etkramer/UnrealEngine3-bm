/**
 * This class is used for linking variables of different types.  It contains a variable of each supported type and can
 * be connected to most types of variables links.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqVar_Union extends SequenceVariable
	native(inherit);

cpptext
{
	virtual INT*		GetIntRef();
	virtual BYTE*		GetByteRef();
	virtual UBOOL*		GetBoolRef();
	virtual FLOAT*		GetFloatRef();
	virtual FString*	GetStringRef();
	virtual UObject**	GetObjectRef( INT Idx );
	virtual FString		GetValueStr();
	virtual struct FUniqueNetId* GetUniqueNetIdRef();

	/**
	 * Union should never be used as the ExpectedType in a variable link, so it doesn't support any property classes.
	 */
	virtual UBOOL SupportsProperty(UProperty *Property);

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

	/**
	 * Allows the sequence variable to execute additional logic after copying values from the SequenceOp's members to the sequence variable.
	 *
	 * @param	SourceOp	the sequence op that contains the value that should be copied to this sequence variable
	 * @param	VarLink		the variable link in Op that this sequence variable is linked to
	 */
	virtual void PostPopulateValue( USequenceOp* SourceOp, FSeqVarLink& VarLink );
}

/**
 * The list of sequence variable classes that are supported by SeqVar_Union
 */
var		array<class<SequenceVariable> >	SupportedVariableClasses;

var()	int			IntValue;
var()	byte		ByteValue;
var()	int			BoolValue;
var()	float		FloatValue;
var()	string		StringValue;
var()	Object		ObjectValue;
var()	UniqueNetId	NetIdValue;

DefaultProperties
{
	ObjName="Union"
	ObjColor=(R=255,G=255,B=255,A=255)		// white

	SupportedVariableClasses.Add(class'SeqVar_Bool')
	SupportedVariableClasses.Add(class'SeqVar_Byte')
	SupportedVariableClasses.Add(class'SeqVar_Int')
	SupportedVariableClasses.Add(class'SeqVar_Object')
	SupportedVariableClasses.Add(class'SeqVar_String')
	SupportedVariableClasses.Add(class'SeqVar_Float')
	SupportedVariableClasses.Add(class'SeqVar_UniqueNetId')
}
