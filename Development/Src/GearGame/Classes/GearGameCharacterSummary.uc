/**
 * Gears2 specific version of UICharacterSummary.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearGameCharacterSummary extends UICharacterSummary;

/**
 * Contains information for this character's profile portrait.
 */
struct strictconfig CharacterPortraitIcon
{
	/** the path name to the texture or material for the portrait */
	var		config		string				ImagePathName;

	/** optional texture coordinates for use when referring to a texture atlas. */
	var		config		TextureCoordinates	Coordinates;
};

/** this character's profile portrait; used in the menus and HUD to represent the player's chosen avatar */
var	config		CharacterPortraitIcon		PortraitIcon;

/**
 * Indicates whether this character represents the leader of the squadron type.
 */
var	config		bool						bIsLeader;

/**
 * An array of path names for gametypes that this character is not allowed to play in.
 */
var	config		array<string>				DisallowedGameTypes;

/** Unlockable enum this character is tied to - this unlockable must be unlocked to use this character */
var config		EGearUnlockable				UnlockableValue;

/* == Delegates == */

/* == Natives == */

/* == Events == */
/**
 * Wrapper for getting a reference to this character's portrait icon.  Handles dynamically loading
 * the portait texture if it hasn't been loaded yet.
 */
event Surface GetPortraitImage()
{
	return Surface(DynamicLoadObject(PortraitIcon.ImagePathName, class'Surface', true));
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
function bool QueryPortraitPropertySupport( Property UnsupportedProperty )
{
	return	UnsupportedProperty != None
		&&	UnsupportedProperty.Name == nameof(PortraitIcon);
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
		if ( PropertyValue.PropertyTag == nameof(PortraitIcon) )
		{
			PropertyValue.PropertyType = DATATYPE_Property;
			PropertyValue.ImageValue = GetPortraitImage();
			PropertyValue.AtlasCoordinates = PortraitIcon.Coordinates;
			bResult = true;
		}
	}

	return bResult;
}

DefaultProperties
{
	CanSupportComplexPropertyType=QueryPortraitPropertySupport
}
