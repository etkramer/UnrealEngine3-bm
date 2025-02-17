/**
 * This class is responsible for providing centralized access to static data about a single unlockable item in Gears2, as well as dynamic
 * data such as whether the unlockable has been unlocked, etc.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUnlockableComboProvider extends GearResourceCombinationProvider;

/**
 * Wrapper for retrieving the discoverable id for the collecatable associated with this provider.
 */
final function EGearUnlockable GetUnlockableId()
{
	local GearUnlockableResourceProvider UnlockableProvider;

	UnlockableProvider = GearUnlockableResourceProvider(StaticDataProvider);
	return UnlockableProvider.UnlockableId;
}


/**
 * Wrapper for determining whether this collectable has been found.
 */
final function bool IsUnlocked()
{
	local GearProfileSettings GearProfile;
	local EGearUnlockable UnlockableId;
	local bool bResult;

	GearProfile = GetGearProfile();
	if ( GearProfile != None )
	{
		UnlockableId = GetUnlockableId();
		bResult = GearProfile.HasUnlockableBeenUnlocked(UnlockableId);
	}

	return bResult;
}

/**
 * Wrapper for determining whether this collectable has been viewed.
 */
final function bool DoesUnlockableNeedViewing()
{
	local GearProfileSettings GearProfile;
	local EGearUnlockable UnlockableId;
	local bool bResult;

	GearProfile = GetGearProfile();
	if ( GearProfile != None )
	{
		UnlockableId = GetUnlockableId();
		bResult = GearProfile.IsUnlockableMarkedForAttract(UnlockableId);
	}

	return bResult;
}

/* === UIDataProvider interface === */
/**
 * Callback to allow script-only child classes to add their own supported tags when GetSupportedDataFields is called.
 *
 * @param	out_Fields	the list of data tags supported by this data store.
 */
event GetSupportedScriptFields( out array<UIDataProviderField> out_Fields )
{
	local UIDataProviderField Field;

	Super.GetSupportedScriptFields(out_Fields);

	Field.FieldTag = 'IsUnlocked';
	Field.FieldType = DATATYPE_Property;
	out_Fields[out_Fields.Length] = Field;

	Field.FieldTag = 'DateUnlocked';
	out_Fields[out_Fields.Length] = Field;

	Field.FieldTag = 'HasUpdatedData';
	out_Fields[out_Fields.Length] = Field;
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
	local int i;
	local bool bResult;

	if ( FieldName == "IsUnlocked" )
	{
		FieldValue.PropertyTag = 'IsUnlocked';
		FieldValue.PropertyType = DATATYPE_Property;

		i = int(IsUnlocked());
		FieldValue.StringValue = string(i);
		FieldValue.ArrayValue[0] = i;
		bResult = true;
	}
	else if ( FieldName == "DateUnlocked" )
	{
		FieldValue.PropertyTag = 'DateFound';
		FieldValue.PropertyType = DATATYPE_Property;

		//@todo - get the date found from the profile
		FieldValue.StringValue = "11/11/2008";
		//FieldValue.ArrayValue[0] = i;
		bResult = true;
	}
	else if ( FieldName == "HasUpdatedData" )
	{
		FieldValue.PropertyTag = 'HasUpdatedData';
		FieldValue.PropertyType = DATATYPE_Property;

		i = int(DoesUnlockableNeedViewing());
		FieldValue.StringValue = string(i);
		FieldValue.ArrayValue[0] = i;
		bResult = true;
	}
	else if ( FieldName == "UnlockableName" )
	{
		if ( FieldValue.StringValue != "" && DoesUnlockableNeedViewing() )
		{
			FieldValue.StringValue @= `NavigationMarkup_NewStuff;
		}
	}

	return bResult || Super.GetFieldValue(FieldName, FieldValue, ArrayIndex);
}

DefaultProperties
{

}
