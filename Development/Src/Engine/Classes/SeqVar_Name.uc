/**
 * Sequence variable for holding a name value.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqVar_Name extends SequenceVariable
	native(inherit);

cpptext
{
	/** Returns the value of this variable */
	FName* GetNameRef()
	{
		return &NameValue;
	}

	/** Returns a string representation of the value of this variable */
	FString GetValueStr()
	{
		return NameValue.ToString();
	}

	/**
	 * Determines whether this variable can be used to contain a value for the specified property.
	 */
	virtual UBOOL SupportsProperty( UProperty *Property )
	{
		if (Cast<UNameProperty>(Property))
		{
			return TRUE;
		}

		UArrayProperty* ArrayProp = Cast<UArrayProperty>(Property);
		if (ArrayProp)
		{
			if (Cast<UNameProperty>(ArrayProp->Inner))
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	/**
	 * Copies the value stored by this SequenceVariable to the SequenceOp member variable that it's associated with.
	 *
	 * @param	Op			the sequence op that contains the value that should be copied from this sequence variable
	 * @param	Property	the property in Op that will receive the value of this sequence variable
	 * @param	VarLink		the variable link in Op that this sequence variable is linked to
	 */
	virtual void PublishValue(USequenceOp* Op, UProperty* Property, FSeqVarLink& VarLink);

	/**
	 * Copy the value from the member variable this VariableLink is associated with to this VariableLink's value.
	 *
	 * @param	Op			the sequence op that contains the value that should be copied to this sequence variable
	 * @param	Property	the property in Op that contains the value to copy into this sequence variable
	 * @param	VarLink		the variable link in Op that this sequence variable is linked to
	 */
	virtual void PopulateValue(USequenceOp* Op, UProperty* Property, FSeqVarLink& VarLink);
}

/** the value of this variable */
var() 	name			NameValue;

defaultproperties
{
	ObjName="Name"
	ObjColor=(R=128,G=255,B=255,A=255)		// aqua
}
