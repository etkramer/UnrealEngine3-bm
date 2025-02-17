/**
 * GameVehicle
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GameVehicle extends SVehicle
	config(Game)
	native
	abstract
	notplaceable;



defaultproperties
{
	bCanBeAdheredTo=TRUE
	bCanBeFrictionedTo=TRUE
}
