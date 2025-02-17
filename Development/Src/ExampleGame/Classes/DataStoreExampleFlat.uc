/**
 * An example of a data store that does not contain any nested data providers.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class DataStoreExampleFlat extends UIDataStore;

var array<string> SourceStringArray;

var	int			IntVar;
var string		StringVar;
var	array<int>	ArrayVar;
var	Surface		ImageVar;
var	UIRangeData	RangeVar;

/**
 * Callback to allow script-only child classes to add their own supported tags when GetSupportedDataFields is called.
 *
 * @param	out_Fields	the list of data tags supported by this data store.
 */
event GetSupportedScriptFields( out array<UIDataProviderField> out_Fields )
{
	local UIDataProviderField TempField;

	// IntVar
	TempField.FieldTag = 'IntVar';
	TempField.FieldType = DATATYPE_Property;
	out_Fields[out_Fields.Length] = TempField;

	// StringVar
	TempField.FieldTag = 'StringVar';
	TempField.FieldType = DATATYPE_Property;
	out_Fields[out_Fields.Length] = TempField;

	// ArrayVar
	TempField.FieldTag = 'ArrayVar';
	TempField.FieldType = DATATYPE_Collection;
	out_Fields[out_Fields.Length] = TempField;

	// ImageVar
	TempField.FieldTag = 'ImageVar';
	TempField.FieldType = DATATYPE_Property;
	out_Fields[out_Fields.Length] = TempField;

	// RangeVar
	TempField.FieldTag = 'RangeVar';
	TempField.FieldType = DATATYPE_RangeProperty;
	out_Fields[out_Fields.Length] = TempField;

	// SourceStringArray
	TempField.FieldTag = 'SourceStringArray';
	TempField.FieldType = DATATYPE_Collection;
	out_Fields[out_Fields.Length] = TempField;
}

/**
 * Resolves the value of the data field specified and stores it in the output parameter.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see ParseDataStoreReference for additional notes
 * @param	ArrayIndex		optional array index for use with data collections
 *
 * @return	TRUE to indicate that this value was processed by script.
 */
event bool GetFieldValue( string FieldName, out UIProviderScriptFieldValue FieldValue, optional int ArrayIndex=INDEX_NONE )
{
	local bool bResult;

	switch ( FieldName )
	{
	case "IntVar":
		FieldValue.StringValue = string(IntVar);
		bResult = true;
		break;
	case "StringVar":
		FieldValue.StringValue = StringVar;
		bResult = true;
		break;
	case "ArrayVar":
		FieldValue.ArrayValue = ArrayVar;
		bResult = true;
		break;
	case "ImageVar":
		FieldValue.ImageValue = ImageVar;
		bResult = true;
		break;
	case "RangeVar":
		FieldValue.RangeValue = RangeVar;
		bResult = true;
		break;
	case "SourceStringArray":
		if ( ArrayIndex >= 0 && ArrayIndex < SourceStringArray.Length )
		{
			FieldValue.StringValue = SourceStringArray[ArrayIndex];
			bResult = true;
		}
		break;
	}

	return bResult;
}

/**
 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
 *
 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
 * @param	FieldValue		the value to store for the property specified.
 * @param	ArrayIndex		optional array index for use with data collections
 *
 * @return	TRUE to indicate that this value was processed by script.
 */
event bool SetFieldValue( string FieldName, const out UIProviderScriptFieldValue FieldValue, optional int ArrayIndex=INDEX_NONE )
{
	local bool bResult;

	switch ( FieldName )
	{
	case "IntVar":
		IntVar = int(FieldValue.StringValue);
		bResult = true;
		break;
	case "StringVar":
		StringVar = FieldValue.StringValue;
		bResult = true;
		break;
	case "ArrayVar":
		ArrayVar = FieldValue.ArrayValue;
		bResult = true;
		break;
	case "ImageVar":
		ImageVar = FieldValue.ImageValue;
		bResult = true;
		break;
	case "RangeVar":
		RangeVar = FieldValue.RangeValue;
		bResult = true;
		break;
	case "SourceStringArray":
		if ( ArrayIndex >= 0 && ArrayIndex < SourceStringArray.Length )
		{
			SourceStringArray[ArrayIndex] = FieldValue.StringValue;
			bResult = true;
		}
		break;
	}

	return bResult;
}

DefaultProperties
{
	Tag="(Example) Flat Data Store"
	WriteAccessType=ACCESS_WriteAll

	IntVar=456789
	StringVar="Flat Data Store Example String"
	ArrayVar(0)=2
	ArrayVar(1)=4
	ImageVar=Texture2D'EngineMaterials.RandomAngles'
	RangeVar=(MinValue=0.f,MaxValue=100.f,NudgeValue=1.f,CurrentValue=50.f)

	SourceStringArray(0)="Source String A"
	SourceStringArray(1)="Source String B"
	SourceStringArray(2)="Source String C"
	SourceStringArray(3)="Source String D"
	SourceStringArray(4)="Source String E"
	SourceStringArray(5)="Source String F"
	SourceStringArray(6)="Source String G"
}
