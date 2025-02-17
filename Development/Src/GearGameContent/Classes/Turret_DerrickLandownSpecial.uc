
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Turret_DerrickLandownSpecial extends Turret_Derrick;

/**
 * Utter hack for G2 ship.
 * This turret is to get around a problem Nash is having where the custom camera FOV values he is setting 
 * keep getting bashed back to default somehow.
 */

defaultproperties
{
	// we don't use this while moving, ok to block
	bBlockActors=TRUE
}
