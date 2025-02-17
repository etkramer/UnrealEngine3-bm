/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//-----------------------------------------------------------
// Browser type for archetype classes
//-----------------------------------------------------------
class GenericBrowserType_Archetype extends GenericBrowserType
	native;

cpptext
{
	virtual void Init();

	/**
	 * Determines whether the specified object is an archetype that should be handled by this generic browser type.
	 *
	 * @param	Object	a pointer to a object with the RF_ArchetypeObject flag
	 *
	 * @return	TRUE if this generic browser type supports to object specified.
	 */
	static UBOOL IsArchetypeSupported( UObject* Object );
}

DefaultProperties
{
	Description="Archetypes"
}
