/**
 * Gears2 specific version of UIWeaponSummary.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearGameWeaponSummary extends UIWeaponSummary;

/**
 * Contains information for this weapon's UI icon
 */
struct strictconfig WeaponIconData
{
	/** the path name to the texture or material for the weapon icon */
	var		config		string				IconPathName;

	/** optional texture coordinates for use when referring to a texture atlas. */
	var		config		TextureCoordinates	Coordinates;
};

/** this weapon's UI icon; used in the menus and HUD to represent the weapon */
var	config				WeaponIconData		WeaponIcon;

/** the id for this weapon in the list of GearProfileSetting's ProfileSettings array */
var	config				EGearWeaponType		ProfileId;

/** the unique id for this weapon */
var	config				byte				WeaponId;

/** controls whether this weapon can be selected as the player's default weapon */
var	config				bool				bIsDefaultWeapon;

/* == Delegates == */

/* == Natives == */

/* == Events == */
/**
 * Wrapper for getting a reference to this weapon's icon.  Handles dynamically loading
 * the texture if it hasn't been loaded yet.
 */
event Surface GetWeaponIcon()
{
	return Surface(DynamicLoadObject(WeaponIcon.IconPathName, class'Surface', true));
}

/* == UnrealScript == */

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Allows script only data stores to indicate whether they'd like to handle a property which is not natively supported.
 *
 * @param	UnsupportedProperty		the property that isn't supported natively
 *
 * @return	TRUE if this data provider wishes to perform custom logic to handle the property.
 */
function bool QueryWeaponIconPropertySupport( Property UnsupportedProperty )
{
	return	UnsupportedProperty != None
		&&	UnsupportedProperty.Name == nameof(WeaponIcon);
}

/* == UIDataProvider interface == */
/**
 * Gets the value for the property specified.  Child classes only need to override this function if it contains data fields
 * which do not correspond to a member property in the class, or if the data corresponds to a complex data type, such as struct,
 * array, etc.
 *
 * @param	PropertyValue	[in] the name of the property to get the value for.
 *							[out] should be filled with the value for the specified property tag.
 * @param	ArrayIndex		optional array index for use with data collections
*
 * @return	return TRUE if either the StringValue or ImageValue fields of PropertyValue were set by script.
 */
event bool GetCustomPropertyValue( out UIProviderScriptFieldValue PropertyValue, optional int ArrayIndex=INDEX_NONE )
{
	local bool bResult;

	bResult = Super.GetCustomPropertyValue(PropertyValue, ArrayIndex);
	if ( !bResult )
	{
		if ( PropertyValue.PropertyTag == nameof(WeaponIcon) )
		{
			PropertyValue.PropertyType = DATATYPE_Property;
			PropertyValue.ImageValue = GetWeaponIcon();
			PropertyValue.AtlasCoordinates = WeaponIcon.Coordinates;
			bResult = true;
		}
	}

	return bResult;
}

DefaultProperties
{
	CanSupportComplexPropertyType=QueryWeaponIconPropertySupport
}
